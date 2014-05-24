/**
  UAV mapping and high-level control.

  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-04-28 (Public Domain)
*/
#ifndef __CYBERALASKA_UAV_CONTROL_H
#define __CYBERALASKA_UAV_CONTROL_H

/**
  North, East, South, and West directions.
  These are used to index into the array of sensor values.
*/
enum {N=0, E=1, S=2, W=3, n_directions}; 

/**
  This is *everything* we send into the students' 
  client-side mapping and control code.
  These values originate on the server (or the simulator).
*/
struct AK_uav_control_sensors {
	std::string state; // current field state (see uav_field.h)
	float mouse_x,mouse_y; // location of user's mouse, in field coordinates
	float x, y; // Last detected x,y location of UAV
	float obstacle[4]; // NESW distances to obstacle (onboard sensors)
	float hiker[4]; // NESW distances to hiker
};

/**
 Student-written mapping and control function.
    The list of inputs are the sensor values;
    outputs are the new control and mapping commands below.
*/
void AK_uav_control(AK_uav_control_sensors &uav);


/** Control command: send the UAV to this location. */
void AK_uav_target(float x,float y);

/** Mapping command: add an obstacle to the map at this location. 
    Obstacles closer than one grid cell apart are merged. */
void AK_add_obstacle(float x,float y);

/** Mapping command: add a detected hiker to the map at this location. 
    Hikers closer than one grid cell apart are merged. */
void AK_add_hiker(float x,float y);


/** Land the UAV, ending the mission.  UAV should be near coordinates 1,1 */
void AK_uav_land();


/********** Server Communication ***********
  By default, we run in local-only simulator mode.
*/

/*
  serverMode:
     'S': simulate only (default)
     'L': talk with local server
     'N': talk with network server
*/
void AK_uav_server(char serverMode);


#endif

