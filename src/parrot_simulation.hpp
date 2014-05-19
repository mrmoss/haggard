#ifndef PARROT_SIMULATION_HPP
#define PARROT_SIMULATION_HPP

#include <math.h>

#include <msl/sprite.hpp>

class parrot_simulation
{
	public:
		bool flying;
		bool emergency;
		double battery;
		double x;
		double y;

		//In degrees.
		double dir;

		parrot_simulation();

		void loop(const double dt);

		void draw(const msl::sprite& body,const msl::sprite& prop,
			const msl::sprite& led,const double scale=1.0);

	private:
		double prop_rotation;
		double prop_rotation_speed;
		double prop_rotation_speed_inc;
		double prop_rotation_speed_max;
};

#endif