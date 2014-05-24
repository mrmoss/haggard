/**
  Neato XV-11 Laser Distance Sensor (LDS) serial data formatting code.
  This version does most of the protocol decoding on the Arduino side.
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-02-21 (Public Domain)
*/
#ifndef __CYBERALASKA__NEATO_SENSOR__H
#define __CYBERALASKA__NEATO_SENSOR__H

#include "../cyberalaska/neato_serial.h"

namespace cyberalaska {
	class neato_sensor_t : public sensor_t {
	public:
		/// 360-degree list of direction reports:
		enum {ndir=360};
		NeatoLDSdir dir[ndir];
		
		neato_sensor_t(const metadata_sensor &metadata_) 
			:sensor_t(metadata_) 
		{
			for (int i=0;i<ndir;i++) dir[i].clear();
		}
	};

};
#endif

