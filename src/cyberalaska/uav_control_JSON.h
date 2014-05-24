/**
  UAV control, implementation of network JSON conversion functions.
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-04-28 (Public Domain)
*/
#ifndef __CYBERALASKA_UAV_CONTROL_JSON_H
#define __CYBERALASKA_UAV_CONTROL_JSON_H

#include "uav_field.h"

/** Convert structures to/from JSON */

AK_uav_field  AK_uav_field_from_JSON (const std::string &jsonString);
AK_uav_control_sensors AK_uav_control_sensors_from_JSON(const std::string &jsonString);

std::string JSON_from_AK_uav_field (const AK_uav_field  &out);
std::string JSON_from_AK_uav_control_sensors(const AK_uav_control_sensors &uav);



#endif

