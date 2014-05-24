#include "pid.hpp"

static const int P=0;
static const int I=1;
static const int D=2;

PID::PID(const float p_gain,const float i_gain,const float d_gain):target(0.0),smoothing(0.0),
	limit(0.0),_d_smooth(0.0),_error_old(0.0),_error_total(0.0)
{
	gains[P]=p_gain;
	gains[I]=i_gain;
	gains[D]=d_gain;
}

float PID::update(const float value)
{
	float error[3]={0,0,0};

	error[P]=target-value;
	error[I]=_error_total;
	error[D]=error[0]-_error_old;

	_d_smooth=error[2]*smoothing+_d_smooth*(1.0-smoothing);

	_error_old=error[P];

	if(_error_total>limit)
		_error_total=limit;
	if(_error_total<-limit)
		_error_total=-limit;

	_error_total+=error[P];

	return gains[P]*error[P]+gains[I]*error[I]+gains[D]*_d_smooth;
}

void PID::reset()
{
	_error_total=0;
}