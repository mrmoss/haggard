//Falconer Header
//	Created By:		Mike Moss
//	Modified On:	05/22/2014

//Required Libraries:
//	avcodec
//	avutil
//	swscale

#ifndef FALCONER_H
#define FALCONER_H

#include "msl/socket.hpp"
#include <string>

extern "C"
{
	typedef unsigned long UINT64_C;
	#include "libavcodec/avcodec.h"
	#include "libswscale/swscale.h"
	#include "libavutil/mem.h"
}

class ardrone
{
	public:
		ardrone(const std::string ip="192.168.1.1",const unsigned short control_port=5556,const unsigned short navdata_port=5554,const unsigned short video_port=5555);
		~ardrone();

		operator bool() const;
		bool good() const;
		bool control_good() const;
		bool navdata_good() const;
		bool video_good() const;

		//In milliseconds.
		bool connect(unsigned int time_out=1000);
		void close();

		void navdata_update();
		void video_update();

		void land();
		void emergency_mode_toggle();
		void takeoff();
		void manuever(float altitude,float pitch,float roll,float yaw);
		void hover();

		void set_level();
		void set_outdoor_mode(const bool outdoor);
		void set_using_shell(const bool on);
		void set_using_brushless_motors(const bool brushless);

		//In mm.
		void set_min_altitude(const int mm);
		void set_max_altitude(const int mm);

		void set_video_feed_front();
		void set_video_feed_bottom();

		unsigned int battery_percent() const;
		bool flying() const;
		bool emergency_mode() const;
		bool low_battery() const;
		bool ultrasonic_enabled() const;
		bool motors_good() const;

		//In degrees.
		float pitch() const;
		float roll() const;
		float yaw() const;

		//In cm.
		int altitude() const;

		uint8_t* video_data() const;

	private:
		ardrone(const ardrone& copy);
		ardrone& operator=(const ardrone& copy);
		unsigned int _count;
		msl::socket _control_socket;
		msl::socket _navdata_socket;
		msl::socket _video_socket;
		unsigned int _battery_percent;
		bool _landed;
		bool _emergency_mode;
		bool _low_battery;
		bool _ultrasonic_enabled;
		bool _video_enabled;
		bool _motors_good;
		float _pitch;
		float _roll;
		float _yaw;
		int _altitude;

		uint8_t* _camera_data;

		AVPacket _av_packet;
		AVCodec* _av_codec;
		AVCodecContext* _av_context;
		AVFrame* _av_camera_cmyk;
		AVFrame* _av_camera_rgb;
};

#endif