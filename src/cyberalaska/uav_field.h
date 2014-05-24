/**
  UAV control, implementation of interaction functions.
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-04-28 (Public Domain)
*/
#ifndef __CYBERALASKA_UAV_CONTROL_UI_H
#define __CYBERALASKA_UAV_CONTROL_UI_H

#include <vector> 
#include <stdexcept> 
#include "cyberalaska/uav_control.h" /* client side stuff */
#include "osl/vec2.h" /* 2D vectors */

/**
  This is *everything* we get back from the students' mapping and control code.
  This same structure is converted to JSON and sent to the web front end.
*/
struct AK_uav_field {
	/**
	 The field state is one of:
	    - "setup": field being set up
	    - "prep": UAV refueling
	    - "ready": ready for pilot to take control
	    - "mission": currently on mission (pilot control)
	    - "done": finished flying mission
	*/
	std::string state; 
	
	vec2 uav; // location for parrot
	
	std::vector<vec2> obstacles; // detected obstacles (nearby obstacles are merged before entering this list)
	std::vector<vec2> hikers; // detected hikers
	
	AK_uav_field() { empty(); }
	void empty() {
		state="setup"; uav=vec2(0,0); 
		obstacles=hikers=std::vector<vec2>(); // empty lists
	}
};

// Create a random field object
void AK_uav_create_field(AK_uav_field &field,int sim_seed_ID);

// Global variable storing the last known control outputs
extern AK_uav_field control_output;

// Utility function returning the vector for this NESW index
inline vec2 dir_to_vec2(int dir) {
	const static vec2 dirs[n_directions]={
		vec2(0,1), vec2(1,0), vec2(0,-1), vec2(-1,0)
	};
	if (dir<0 || dir>=n_directions) throw std::runtime_error("Invalid direction in dir_to_index!");
	return dirs[dir];
}

// Check for the nearest target in this list, to build simulated sensor values.
//  Returns a distance between 0.2 and 2.0, or 1000.0 if nothing is in range.
float AK_uav_simulate_sensor(const vec2 &loc,int dir,std::vector<vec2> &list);


// Generate a nice round random number between 0 and range
inline float randfloat(float range) {
	return (rand()%10000)*(1.0/10000.0)*range;
}
// Generate a 2D vector between 0 and range on both axes
inline vec2 randvec(float range) {
	return vec2(randfloat(range),randfloat(range));
}


/// Simulator, for testing student code
class AK_uav_simulator : public AK_uav_field {
public:
	AK_uav_control_sensors sensors; // simulated values, sent to control code
	
	AK_uav_simulator(int sim_seed_ID);
	void step(const vec2 &target,double dt);
};


#endif

