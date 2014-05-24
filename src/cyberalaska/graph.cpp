#include "graph.hpp"

graph::graph(const unsigned int size,const float default_value):_size(size)
{
	for(unsigned int ii=0;ii<_size;++ii)
		_data.push_back(default_value);
}

void graph::add(const float& element)
{
	_data.insert(_data.begin(),element);
	_data.resize(_size);
}

unsigned int graph::size() const
{
	return _size;
}

void graph::draw(const double x,const double y,const msl::color& line_color) const
{
	for(unsigned int ii=0;ii<_data.size()-1;++ii)
		msl::draw_line(x+ii,y+_data[ii],x+ii+1,y+_data[ii+1],line_color);
}