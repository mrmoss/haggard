/**
 Talks to Arduino sensor on the UAV.

 Orion Lawlor & Mike Moss, 2014-05-07 (Public Domain)
*/
#ifndef __CYBERALASKA_UAV_SENSOR_COMMS_H
#define __CYBERALASKA_UAV_SENSOR_COMMS_H
#include "msl/serial.hpp"
#include <string>
#include <vector>

#define UAV_SENSOR_COMMS_NUM_SENSORS 4


/**
 Reads data from UAV over serial port.
*/
class UAV_sensor_comms : public msl::serial {
public:
	UAV_sensor_comms(const std::string &portName,int baud);

	// Perform serial communication
	void update();

	// Return the number of sensors
	unsigned int num_sensors(void) const { return UAV_SENSOR_COMMS_NUM_SENSORS; }

	// Read one sensor value
	float read_sensor(unsigned int sensorNo) const
	{
		if(sensorNo<num_sensors())
			return filtered_data[sensorNo];

		return 0.0;
	}

private:
	enum state_t
	{
		HEADER,
		DATA,
		CRC
	};

	state_t state=HEADER;
	uint8_t crc=0x00;
	std::string buffer; // incoming (unchecked) data
	std::vector<std::vector<float>> unfiltered_data;
	float filtered_data[UAV_SENSOR_COMMS_NUM_SENSORS];
	unsigned long filter_timer;
	unsigned long filter_update_time=50; // milliseconds lag
};

#endif

