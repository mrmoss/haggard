/**
  Simulated UAV, used on both client and server for testing/debugging.

  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-04-28 (Public Domain)
*/
#include "cyberalaska/uav_field.h"
#include <cstdlib>


AK_uav_simulator::AK_uav_simulator(int sim_seed_ID)
{
	AK_uav_create_field(*this,sim_seed_ID);
}

void AK_uav_simulator::step(const vec2 &target,double dt)
{
	// Move UAV
	double speed=2.0;  // max move speed, ft/sec
	double windspeed=1.6; // wind speed, ft/sec
	static vec2 wind_dir=vec2(0.0);
	static double wind_time=0.0;
	wind_time+=dt;
	if (wind_time>1.0) // wind direction changes
	{
		wind_dir=randvec(2.0*windspeed)-vec2(windspeed,windspeed);
		wind_time=0.0;
	}

	vec2 move_dir=target-uav;
	if (length(move_dir)>0.001) move_dir=normalize(move_dir);

	uav+=dt*(wind_dir+speed*move_dir);
	sensors.x=uav.x;
	sensors.y=uav.y;

	// Update simulated sensors
	for (int dir=0;dir<n_directions;dir++) {
		sensors.obstacle[dir]=AK_uav_simulate_sensor(uav,dir,obstacles);
		sensors.hiker[dir]=AK_uav_simulate_sensor(uav,dir,hikers);
	}

	// Check for crash
	for (unsigned int i=0;i<obstacles.size();i++)
	{
		float crash_range=0.9; // UAV + obstacle radius, in feet
		if (length(obstacles[i]-uav)<crash_range) {
			throw std::runtime_error("Crashed into obstacle!");
		}
	}
}

