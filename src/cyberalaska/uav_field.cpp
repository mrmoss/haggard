/**
  Accepts client's AK_uav, AK_add_obstacle, etc commands.

  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-04-28 (Public Domain)
*/
#include "cyberalaska/uav_field.h"
#include <cstdio>

int window_size=700; // pixels across
double field_size=10.0; // feet across


// Detected objects closer than this distance are merged
const float object_merge_dist=1.5;

// Add this newly-detect object to this list of existing objects,
//  merging if needed.
bool merge_into_list(const vec2 &p,std::vector<vec2> &list)
{
	bool merged=false;
	for (unsigned int i=0;i<list.size();i++)
		if (length(p-list[i])<object_merge_dist)
		{
			if (!merged) { // update old location
				list[i]=p;
				merged=true;
			} else { // delete stale copy
				list.erase(list.begin()+i);
				i--;
			}
		}
	if (!merged)
		list.push_back(p);
	return merged;
}


// Check for the nearest target in this list, to build simulated sensor values.
//  Returns a distance between 0.2 and 2.0, or 1000.0 if nothing is in range.
float AK_uav_simulate_sensor(const vec2 &loc,int dir,std::vector<vec2> &list)
{
	static float dist_threshold=2.0; // feet sensor range
	static float angle_threshold=20; // degrees half field of view
	static float cos_threshold=cos(angle_threshold*M_PI/180.0);

	vec2 d=dir_to_vec2(dir);
	float closest=1000.0;
	for (unsigned int i=0;i<list.size();i++)
	{
		vec2 rel=list[i]-loc;
		float dist=length(rel);
		float cosAng=dot(rel,d)/(length(rel)*length(d)); // cosine of angle from sensor to target

		if (dist<dist_threshold && dist>0.2) // within sensor range
		if (cosAng>cos_threshold) // in sensor field of view
		{
			dist+=randfloat(0.1); // sensor noise
			closest=std::min(closest,dist);
		}
	}
	return closest;
}


// Global variable storing the last known control outputs
AK_uav_field control_output;

/** Control command: send the UAV to this location. */
void AK_uav_target(float x,float y)
{
	if (control_output.state=="ready")
		control_output.state="mission"; // mission starts when we set the target
	if (control_output.state=="mission")
		control_output.uav=vec2(x,y); // set new target position for UAV
}

/** Land the UAV, ending the mission.  UAV should be near coordinates 1,1 */
void AK_uav_land()
{
	if (control_output.state=="mission")
		control_output.state="land";
}

/** Mapping command: add an obstacle to the map at this location.
    Obstacles closer than one grid cell apart are merged. */
void AK_add_obstacle(float x,float y)
{
	merge_into_list(vec2(x,y),control_output.obstacles);
}


/** Mapping command: add a detected hiker to the map at this location.
    Hikers closer than one grid cell apart are merged. */
void AK_add_hiker(float x,float y)
{
	merge_into_list(vec2(x,y),control_output.hikers);
}




/************* Random Field Generator **************/
const static float field_closest=2.5; // minimum distance between objects on the field
const static float field_edge=1.0; // minimum distance to edge of field

// Generate a random point on the field
static vec2 rand_field(void) {
	return randvec(field_size-2.0*field_edge)+vec2(field_edge,field_edge);
}

// return true if this point is near a previous point on the list
bool point_near(const vec2 &p,const std::vector<vec2> &list)
{
	for (unsigned int i=0;i<list.size();i++)
		if (length(p-list[i])<field_closest) return true;
	return false;
}

// Create a random field object
void AK_uav_create_field(AK_uav_field &field,int sim_seed_ID)
{
	srand(sim_seed_ID);
	field.state="setup";
	field.uav=vec2(0.0,0.0); // takeoff position
	field.hikers=field.obstacles=std::vector<vec2>(); // clear lists

	int nobs=2;
	for (int o=0;o<nobs;o++) // obstacles
	{
		vec2 p=rand_field();
		if (length(p-field.uav)<field_closest  // near origin
			|| point_near(p,field.obstacles)) o--; // try again
		else field.obstacles.push_back(p);
	}

	int nhiker=2+(rand()%2);
	for (int h=0;h<nhiker;h++) // hikers
	{
		vec2 p=rand_field();
		if (point_near(p,field.hikers) // near another hiker
		 || point_near(p,field.obstacles)) // near an obstacle
		{
			h--; // try a different point
		}
		else field.hikers.push_back(p);
	}

}



