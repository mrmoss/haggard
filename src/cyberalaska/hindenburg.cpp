#include "hindenburg.hpp"

#include "msl/time_util.hpp"

hindenburg::hindenburg(const std::string& port,const unsigned int baud):
	alt_hold(false),leveler(0,0,0),serial(port,baud),timer(0),pitch_center(1500),
	roll_center(1500),throttle_center(1300),yaw_center(1500),flags(0x06),
	pitch(pitch_center),roll(roll_center),throttle(throttle_center),
	yaw(yaw_center),altitude(0),altitude_limit(0)
{}

void hindenburg::connect()
{
	serial.connect();
}

bool hindenburg::good() const
{
	return serial.good();
}

bool hindenburg::armed() const
{
	return (flags&0x01);
}

void hindenburg::set_armed(const bool enable)
{
	if(enable)
		arm();
	else
		disarm();
}

void hindenburg::arm()
{
	flags|=0x01;
}

void hindenburg::disarm()
{
	flags&=~0x01;
}

void hindenburg::maneuver(const uint16_t new_pitch,const uint16_t new_roll,const uint16_t new_yaw)
{
	pitch=new_pitch;
	roll=new_roll;
	yaw=new_yaw;
}

void hindenburg::send_pids(const uint8_t pitch_pid[3],const uint8_t roll_pid[3],
	const uint8_t throttle_pid[3],const uint8_t yaw_pid[3])
{
	std::string packet=serial_pack_pid(pitch_pid,roll_pid,throttle_pid,yaw_pid);
	serial.write(packet.c_str(),packet.size());
}

void hindenburg::update()
{
	uint16_t throttle_send=throttle;

	if(throttle>1500)
		throttle=1500;
	if(throttle<1000)
		throttle=1000;

	if(armed())
	{
		if(alt_hold&&msl::millis()>=timer)
		{
			throttle_send=throttle_center+leveler.update(altitude);

			if(throttle>throttle_center+100)
				throttle=throttle_center+100;
			if(throttle<throttle_center-100)
				throttle=throttle_center-100;

			timer=msl::millis()+10;
		}

		if(throttle_send>1500)
			throttle_send=1500;
		if(throttle_send<1000)
			throttle_send=1000;
	}
	else
	{
		throttle=1000;
		leveler.reset();
	}

	std::string packet=serial_pack_maneuver(flags,pitch,roll,throttle_send,yaw);
	serial.write(packet.c_str(),packet.size());

	uint8_t b;

	while(serial.available()>0&&serial.read(&b,1)==1)
		altitude=b;

	if(altitude>altitude_limit)
		altitude=altitude_limit;
	if(altitude<0)
		altitude=0;
}

uint8_t hindenburg::serial_checksum(const void* buffer,const uint16_t size)
{
	uint8_t checksum=0;

	for(uint16_t ii=0;ii<size;++ii)
		checksum^=((uint8_t*)buffer)[ii];

	return checksum;
}

std::string hindenburg::serial_pack_maneuver(const uint8_t status,const uint16_t pitch,
	const uint16_t roll,const uint16_t throttle,const uint16_t yaw)
{
	//Header
	std::string packet("m");

	//Payload Size
	packet+=(char)0x0A;

	//Type is Maneuver
	packet+=(char)0x00;

	//Status Byte
	packet+=status;

	//Maneuver
	for(int ii=0;ii<2;++ii)
		packet+=((uint8_t*)&pitch)[ii];
	for(int ii=0;ii<2;++ii)
		packet+=((uint8_t*)&roll)[ii];
	for(int ii=0;ii<2;++ii)
		packet+=((uint8_t*)&throttle)[ii];
	for(int ii=0;ii<2;++ii)
		packet+=((uint8_t*)&yaw)[ii];

	//Checksum
	packet+=serial_checksum(packet.c_str(),packet.size());

	//Return Packet
	return packet;
}

std::string hindenburg::serial_pack_pid(const uint8_t pitch_pid[3],const uint8_t roll_pid[3],
	const uint8_t throttle_pid[3],const uint8_t yaw_pid[3])
{
	//Header
	std::string packet("m");

	//Payload Size
	packet+=(char)0x0D;

	//Type is Set PID
	packet+=(char)0x01;

	//Set PIDs
	for(int ii=0;ii<3;++ii)
		packet+=pitch_pid[ii];
	for(int ii=0;ii<3;++ii)
		packet+=roll_pid[ii];
	for(int ii=0;ii<3;++ii)
		packet+=throttle_pid[ii];
	for(int ii=0;ii<3;++ii)
		packet+=yaw_pid[ii];

	//Checksum
	packet+=serial_checksum(packet.c_str(),packet.size());

	//Return Packet
	return packet;
}