/**
 Talks to Arduino sensor on the UAV.

 Orion Lawlor & Mike Moss, 2014-05-07 (Public Domain)
*/
#include <iostream>
#include "cyberalaska/uav_sensor_comms.h"
#include "msl/time_util.hpp"
#include <math.h>


UAV_sensor_comms::UAV_sensor_comms(const std::string &portName,int baud)
	:msl::serial(portName,baud),
	state(HEADER),
	crc(0),filtered_data{0,0,0,0},filter_timer(msl::millis()+filter_update_time)
{
	for(unsigned int ii=0;ii<num_sensors();++ii)
		unfiltered_data.push_back({});
}


void UAV_sensor_comms::update()
{
	msl::serial &port=*this; // silly hack; port should maybe be a member
	uint8_t temp;

	while(port.available()>0&&port.read(&temp,1)==1)
	{
		if(state==HEADER&&temp=='m')
		{
			crc=0x00;
			crc^=temp;
			state=DATA;
		}
		else if(state==DATA)
		{
			buffer+=temp;
			crc^=temp;

			if(buffer.size()>=4)
				state=CRC;
		}
		else if(state==CRC)
		{
			if(crc==temp) // crc matches!
			{
				for(unsigned int ii=0;ii<num_sensors();++ii)
					unfiltered_data[ii].push_back((int)buffer[ii]);
			}
			else {
				std::cout<<"UAV sensor CRC ERROR!\n";
			}

			state=HEADER;
			buffer="";
			crc=0x00;
		}
	}

	if(msl::millis()>=filter_timer)
	{
		const char *dir_names[4]={"N","E","S","W"};
		float max_dist=10; // centimeter variation
		unsigned int sample_size=5;

		for(unsigned int ii=0;ii<num_sensors();++ii)
		{
			if(unfiltered_data[ii].size()>sample_size)
			{
				//Calculate raw average
				float raw_average=0;

				for(auto val:unfiltered_data[ii])
					raw_average+=val;

				raw_average/=(float)unfiltered_data[ii].size();

				//Filter out outliers
				for(unsigned int jj=0;jj<unfiltered_data[ii].size();++jj)
				{
					if(fabs(unfiltered_data[ii][jj]-raw_average)>max_dist)
					{
						unfiltered_data[ii].erase(unfiltered_data[ii].begin()+jj);
						--jj;
					}
				}

				//Calculate new average
				float new_average=0;

				if(unfiltered_data[ii].size()>sample_size)
				{
					for(auto val:unfiltered_data[ii])
					new_average+=val;

					new_average/=(float)unfiltered_data[ii].size();
				}

				filtered_data[ii]=new_average;

				unfiltered_data[ii].clear();

				std::cout<<"sensor["<<dir_names[ii]<<"]="<<filtered_data[ii]<<" cm\tavg="<<new_average<<std::endl;
			}
		}

		filter_timer=msl::millis()+filter_update_time;
	}
}



