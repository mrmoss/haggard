#ifndef HINDENBURG_HPP
#define HINDENBURG_HPP

#include "pid.hpp"
#include "msl/serial.hpp"
#include <stdint.h>

class hindenburg
{
	public:
		hindenburg(const std::string& port="",const unsigned int baud=0);

		void connect();

		bool good() const;

		bool armed() const;
		void set_armed(const bool enable);
		void arm();
		void disarm();

		void maneuver(const uint16_t pitch,const uint16_t roll,const uint16_t yaw);

		void send_pids(const uint8_t pitch_pid[3],const uint8_t roll_pid[3],
			const uint8_t throttle_pid[3],const uint8_t yaw_pid[3]);

		void update();

		bool alt_hold;
		PID leveler;

		//Things that shouldn't really be touched...
		msl::serial serial;
		unsigned long timer;

		uint16_t pitch_center;
		uint16_t roll_center;
		uint16_t throttle_center;
		uint16_t yaw_center;
		uint8_t flags;

		uint16_t pitch;
		uint16_t roll;
		uint16_t throttle;
		uint16_t yaw;

		uint8_t altitude;
		uint8_t altitude_limit;

		uint8_t serial_checksum(const void* buffer,const uint16_t size);

		std::string serial_pack_maneuver(const uint8_t status,const uint16_t pitch,
			const uint16_t roll,const uint16_t throttle,const uint16_t yaw);

		std::string serial_pack_pid(const uint8_t pitch_pid[3],const uint8_t roll_pid[3],
			const uint8_t throttle_pid[3],const uint8_t yaw_pid[3]);
};

#endif