#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "msl/2d_util.hpp"
#include <vector>

class graph
{
	public:
		graph(const unsigned int size,const float default_value=0.0);
		void add(const float& element);
		unsigned int size() const;
		void draw(const double x,const double y,const msl::color& line_color) const;

	//private:
		std::vector<float> _data;
		unsigned int _size;
};

#endif