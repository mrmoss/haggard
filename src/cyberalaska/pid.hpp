#ifndef PID_HPP
#define PID_HPP

class PID
{
	public:
		PID(const float p_gain,const float i_gain,const float d_gain);

		float update(const float value);

		void reset();

		float gains[3];
		float target;
		float smoothing;
		float limit;

	private:
		float _d_smooth;
		float _error_old;
		float _error_total;
};

#endif