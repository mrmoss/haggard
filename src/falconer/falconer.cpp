//Falconer Source
//	Created By:		Mike Moss
//	Modified On:	05/22/2014

//Required Libraries:
//	avcodec
//	avutil
//	swscale

//Definitions for "falconer.hpp"
#include "falconer.hpp"

//Algorithm Header
#include <algorithm>

//Exceptions Header
#include <stdexcept>

//C String Header
#include <string.h>

//String Utility Header
#include "msl/string_util.hpp"

//Time Utility Header
#include "msl/time_util.hpp"

//https://github.com/elliotwoods/ARDrone-GStreamer-test/blob/master/plugin/src/pave.h
struct parrot_video_encapsulation_t
{
	uint8_t signature[4];
	uint8_t version;
	uint8_t video_codec;
	uint16_t header_size;
	uint32_t payload_size;					/* Amount of data following this PaVE */
	uint16_t encoded_stream_width;			/* ex: 640 */
	uint16_t encoded_stream_height;			/* ex: 368 */
	uint16_t display_width;					/* ex: 640 */
	uint16_t display_height;				/* ex: 360 */
	uint32_t frame_number;					/* frame position inside the current stream */
	uint32_t timestamp;						/* in milliseconds */
	uint8_t total_chuncks;					/* number of UDP packets containing the current decodable payload */
	uint8_t chunck_index ;					/* position of the packet - first chunk is #0 */
	uint8_t frame_type;						/* I-frame, P-frame */
	uint8_t control;						/* Special commands like end-of-stream or advertised frames */
	uint32_t stream_byte_position_lw;		/* Byte position of the current payload in the encoded stream - lower 32-bit word */
	uint32_t stream_byte_position_uw;		/* Byte position of the current payload in the encoded stream - upper 32-bit word */
	uint16_t stream_id;						/* This ID indentifies packets that should be recorded together */
	uint8_t total_slices;					/* number of slices composing the current frame */
	uint8_t slice_index ;					/* position of the current slice in the frame */
	uint8_t header1_size;					/* H.264 only : size of SPS inside payload - no SPS present if value is zero */
	uint8_t header2_size;					/* H.264 only : size of PPS inside payload - no PPS present if value is zero */
	uint8_t reserved2[2];					/* Padding to align on 48 bytes */
	uint32_t advertised_size;				/* Size of frames announced as advertised frames */
	uint8_t reserved3[12];					/* Padding to align on 64 bytes */
};

ardrone::ardrone(const std::string ip,const unsigned short control_port,const unsigned short navdata_port,const unsigned short video_port):
	_count(1),
	_control_socket(ip+":"+msl::to_string(control_port)),
	_navdata_socket(ip+":"+msl::to_string(navdata_port)),
	_video_socket(ip+":"+msl::to_string(video_port)),
	_battery_percent(0),
	_landed(true),
	_emergency_mode(false),
	_low_battery(false),
	_ultrasonic_enabled(false),
	_video_enabled(false),
	_motors_good(false),
	_pitch(0),_roll(0),_yaw(0),_altitude(0)
{
	//Hide Libav debug output...can't really print anything else...
	av_log_set_level(AV_LOG_QUIET);

	//Allocate camera data and clear to zero.
	_camera_data=new uint8_t[640*368*3];
	memset(_camera_data,0,640*368*3);

	//Register all the codecs, parsers and bitstream filters which were enabled at configuration time...
	avcodec_register_all();

	//Initialize codec frame packets and buffers.
	memset(&_av_packet,0,sizeof(_av_packet));
	av_init_packet(&_av_packet);
	_av_packet.data=new uint8_t[100000];
	memset(_av_packet.data,0,100000);

	//Find a codec.
	_av_codec=avcodec_find_decoder(CODEC_ID_H264);

	//Failed to find codec, cleanup and throw.
	if(!_av_codec)
	{
		delete[] _camera_data;
		delete[] _av_packet.data;
		throw std::runtime_error("ardrone::ardrone() - Could not find a video decoder (CODEC_ID_H264)!");
	}

	//Found codec, allocate frame data.
	_av_context=avcodec_alloc_context3(_av_codec);
	_av_camera_cmyk=avcodec_alloc_frame();
	_av_camera_rgb=avcodec_alloc_frame();

	//Open codec, on failure, cleanup and throw.
	if(avcodec_open2(_av_context,_av_codec,NULL)<0)
	{
		delete[] _camera_data;
		avcodec_close(_av_context);
		av_free(_av_context);
		av_free(_av_camera_cmyk);
		av_free(_av_camera_rgb);
		delete[] _av_packet.data;
	}
}

ardrone::~ardrone()
{
	_control_socket.close();
	_navdata_socket.close();

	_video_socket.close();
	delete[] _camera_data;
	avcodec_close(_av_context);
	av_free(_av_context);
	av_free(_av_camera_cmyk);
	av_free(_av_camera_rgb);
	delete[] _av_packet.data;
}

ardrone::operator bool() const
{
	return good();
}

bool ardrone::good() const
{
	bool ret=(control_good()&&navdata_good()&&video_good());

	return ret;
}

bool ardrone::control_good() const
{
	return _control_socket;
}

bool ardrone::navdata_good() const
{
	return _navdata_socket;
}

bool ardrone::video_good() const
{
	return _video_socket;
}

bool ardrone::connect(unsigned int time_out)
{
	//Connect to sockets.
	if(!_control_socket.good())
		_control_socket.connect_udp();
	if(!_navdata_socket.good())
		_navdata_socket.connect_udp();
	if(!_video_socket.good())
		_video_socket.connect_tcp();

	//Wait 1 second for the connection to establish...
	msl::nsleep(1000000000);

	//If connected, set some settings...
	if(good())
	{
		//Reset counter.
		_count=1;

		//Turn on full navdata packets.
		std::string navdata_enable_command="AT*CONFIG="+msl::to_string(_count)+
			",\"general:navdata_demo\",\"FALSE\"\r";
		++_count;
		_control_socket.write(navdata_enable_command);

		//Send all navdata options.
		std::string navdata_send_all_command="AT*CONFIG="+msl::to_string(_count)+
			",\"general:navdata_options\",\"65537\"\r";
		++_count;
		_control_socket.write(navdata_send_all_command);

		//Set the watchdog timer.
		std::string watchdog_command="AT*COMWDG="+msl::to_string(_count)+"\r";
		++_count;
		_control_socket.write(watchdog_command);

		//Set the video codec.
		std::string video_codec_command="AT*CONFIG="+msl::to_string(_count)+
			",\"video:video_codec\",\"P264_CODEC\"\r";
		++_count;
		_control_socket.write(video_codec_command);

		//Set the video codec speed.
		std::string video_codec_speed_command="AT*CONFIG="+msl::to_string(_count)+
			",\"video:codec_fps\",\"30\"\r";
		++_count;
		_control_socket.write(video_codec_speed_command);

		return true;
	}

	return false;
}

void ardrone::close()
{
	_control_socket.close();
	_navdata_socket.close();
	_video_socket.close();
}

void ardrone::navdata_update()
{
	if(good())
	{
		char redirect_navdata_command[14]={1,0,0,0,0,0,0,0,0,0,0,0,0,0};
		_navdata_socket.write(redirect_navdata_command,14);

		const int packet_size=500;			//nav-data-full packet size=500, nav-data-demo packet size=24
		uint8_t byte[packet_size];

		if(_navdata_socket.available()>0&&_navdata_socket.read(byte,packet_size,200)==packet_size)
		{
			unsigned int packet_header;
			memcpy(&packet_header,byte,4);

			if(packet_header==0x55667788)
			{
				unsigned int states=0;
				memcpy(&states,byte+4,4);

				_landed=!static_cast<bool>(states&(1<<0));
				_emergency_mode=static_cast<bool>(states&(1<<31));
				_low_battery=static_cast<bool>(states&(1<<15));
				_ultrasonic_enabled=!static_cast<bool>(states&(1<<21));
				_video_enabled=static_cast<bool>(states&(1<<1));
				_motors_good=!static_cast<bool>(states&(1<<12));

				unsigned short header=-1;
				memcpy(&header,byte+16,2);

				if(header==0)
				{
					memcpy(&_battery_percent,byte+24,4);
					memcpy(&_pitch,byte+28,4);
					memcpy(&_roll,byte+32,4);
					memcpy(&_yaw,byte+36,4);
					memcpy(&_altitude,byte+40,4);
				}
			}
		}
	}
}

void ardrone::video_update()
{
	if(good())
	{
		char video_keepalive_command[1]={1};
		_video_socket.write(&video_keepalive_command,1);

		parrot_video_encapsulation_t video_packet;
		memset(&video_packet,0,sizeof(video_packet));
		_av_packet.size=_video_socket.read(_av_packet.data,sizeof(parrot_video_encapsulation_t),200);

		if(static_cast<unsigned int>(_av_packet.size)<=sizeof(video_packet))
		{
			memcpy(&video_packet,_av_packet.data,_av_packet.size);
			_av_packet.size=_video_socket.read(_av_packet.data,video_packet.payload_size,200);

			_av_packet.flags=0;

			if(video_packet.frame_type==1)
				_av_packet.flags=AV_PKT_FLAG_KEY;

			int frame_decoded=0;

			if(avcodec_decode_video2(_av_context,_av_camera_cmyk,&frame_decoded,&_av_packet)>0&&frame_decoded>0)
			{
				if(video_packet.encoded_stream_width==640&&video_packet.encoded_stream_height==368)
				{
					SwsContext* _sws_context=sws_getContext(video_packet.encoded_stream_width,video_packet.encoded_stream_height,PIX_FMT_YUV420P,video_packet.encoded_stream_width,
						video_packet.encoded_stream_height,PIX_FMT_RGB24,SWS_BICUBIC,NULL,NULL,NULL);

					if(_sws_context!=NULL)
					{
						if(_av_camera_rgb!=NULL&&_camera_data!=NULL)
							avpicture_fill(reinterpret_cast<AVPicture*>(_av_camera_rgb),_camera_data,PIX_FMT_BGR24,video_packet.encoded_stream_width,video_packet.encoded_stream_height);

						if(_av_camera_cmyk->data!=NULL&&video_packet.display_width==640&&video_packet.display_height==360)
							sws_scale(_sws_context,_av_camera_cmyk->data,_av_camera_cmyk->linesize,0,video_packet.display_height,_av_camera_rgb->data,_av_camera_rgb->linesize);

						sws_freeContext(_sws_context);
					}
				}
			}
		}
	}
}

void ardrone::land()
{
	if(good())
	{
		int land_flags=1<<18|1<<20|1<<22|1<<24|1<<28;
		std::string command="AT*REF="+msl::to_string(_count)+","+msl::to_string(land_flags)+"\r";
		++_count;
		_control_socket.write(command);
	}
}

void ardrone::emergency_mode_toggle()
{
	if(good())
	{
		int emergency_flags=1<<8|1<<18|1<<20|1<<22|1<<24|1<<28;
		std::string command="AT*REF="+msl::to_string(_count)+","+msl::to_string(emergency_flags)+"\r";
		++_count;
		_control_socket.write(command);
	}
}

void ardrone::takeoff()
{
	if(good())
	{
		int takeoff_flags=1<<9|1<<18|1<<20|1<<22|1<<24|1<<28;
		std::string command="AT*REF="+msl::to_string(_count)+","+msl::to_string(takeoff_flags)+"\r";
		++_count;
		_control_socket.write(command);
	}
}

void ardrone::manuever(float altitude,float pitch,float roll,float yaw)
{
	if(good())
	{
		altitude=std::max((float)-1.0,std::min((float)1.0,altitude));
		pitch=std::max((float)-1.0,std::min((float)1.0,pitch));
		roll=std::max((float)-1.0,std::min((float)1.0,roll));
		yaw=std::max((float)-1.0,std::min((float)1.0,yaw));

		std::string command="AT*PCMD="+msl::to_string(_count)+",1,"+msl::to_string(*(int*)(&roll))+","+msl::to_string(*(int*)(&pitch))
			+","+msl::to_string(*(int*)(&altitude))+","+msl::to_string(*(int*)(&yaw))+"\r";
		++_count;
		_control_socket.write(command);
	}
}

void ardrone::hover()
{
	if(good())
	{
		std::string command="AT*PCMD="+msl::to_string(_count)+",0,0,0,0,0\r";
		++_count;
		_control_socket.write(command);
	}
}

void ardrone::set_level()
{
	std::string initialize_command="AT*FTRIM="+msl::to_string(_count)+"\r";
	++_count;
	_control_socket.write(initialize_command);
}

void ardrone::set_outdoor_mode(const bool outdoor)
{
	std::string bool_value="FALSE";

	if(outdoor)
		bool_value="TRUE";

	std::string outdoor_hull_command="AT*CONFIG="+msl::to_string(_count)+",\"control:outdoor\",\""+bool_value+"\"\r";
	++_count;
	_control_socket.write(outdoor_hull_command);
}

void ardrone::set_using_shell(const bool on)
{
	std::string bool_value="FALSE";

	if(on)
		bool_value="TRUE";

	std::string shell_is_on_command="AT*CONFIG="+msl::to_string(_count)+",\"control:flight_without_shell\",\""+bool_value+"\"\r";
	++_count;
	_control_socket.write(shell_is_on_command);
}

void ardrone::set_using_brushless_motors(const bool brushless)
{
	std::string bool_value="FALSE";

	if(brushless)
		bool_value="TRUE";

	std::string motor_type_command="AT*CONFIG="+msl::to_string(_count)+
		",\"control:brushless\",\""+bool_value+"\"\r";
	++_count;
	_control_socket.write(motor_type_command);
}

void ardrone::set_min_altitude(const int mm)
{
	std::string altitude_min_command="AT*CONFIG="+msl::to_string(_count)+
		",\"control:altitude_min\",\""+msl::to_string(mm)+"\"\r";
	++_count;
	_control_socket.write(altitude_min_command);
}

void ardrone::set_max_altitude(const int mm)
{
	std::string altitude_max_command="AT*CONFIG="+msl::to_string(_count)+
		",\"control:altitude_max\",\""+msl::to_string(mm)+"\"\r";
	++_count;
	_control_socket.write(altitude_max_command);
}

void ardrone::set_video_feed_front()
{
	if(good())
	{
		std::string command="AT*CONFIG="+msl::to_string(_count)+
			",\"video:video_channel\",\"2\"\r";
		++_count;
		_control_socket.write(command);
	}
}

void ardrone::set_video_feed_bottom()
{
	if(good())
	{
		std::string command="AT*CONFIG="+msl::to_string(_count)+
			",\"video:video_channel\",\"3\"\r";
		++_count;
		_control_socket.write(command);
	}
}

unsigned int ardrone::battery_percent() const
{
	return _battery_percent;
}

bool ardrone::flying() const
{
	return !_landed;
}

bool ardrone::emergency_mode() const
{
	return _emergency_mode;
}

bool ardrone::low_battery() const
{
	return _low_battery;
}

bool ardrone::ultrasonic_enabled() const
{
	return _ultrasonic_enabled;
}

bool ardrone::motors_good() const
{
	return _motors_good;
}

int ardrone::altitude() const
{
	return _altitude;
}

//command gives millidegrees, convert to degrees.
float ardrone::pitch() const
{
	return _pitch/1000.0;
}

//command gives millidegrees, convert to degrees.
float ardrone::roll() const
{
	return _roll/1000.0;
}

//command gives millidegrees, convert to degrees.
float ardrone::yaw() const
{
	return _yaw/1000.0;
}

uint8_t* ardrone::video_data() const
{
	return _camera_data;
}
