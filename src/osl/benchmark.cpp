/*
Performance statistics collection.

Orion Sky Lawlor, olawlor@acm.org, 2004/8/18
*/
#include <stdio.h>
#include "osl/benchmark.h"

using benchmark::op_t;

// benchmark::get
static benchmark::stats staticStats;

benchmark::stats *benchmark::get(void)
{
	return &staticStats;
}

// benchmark::op_t registration
namespace benchmark { 
class op_info_t {
public:
	const char *name; // Short human-readable no-spaces name.
	const char *desc; // Long human-readable description.
	bool isTime; // if true, this is a timing field.
	const char *units; // Human-readable units, like "seconds" or "bytes"
}; 
};
static benchmark::op_info_t op_info[benchmark::op_max];
int benchmark::op_len; /* <- implicitly zeroed at startup */
static op_t addOp(const benchmark::op_info_t &i) {
	if (benchmark::op_len==benchmark::op_max) {
		osl::bad("Registered too many osl::benchmark operations!\n");
	}
	op_info[benchmark::op_len]=i;
	op_t ret;
	ret.idx=benchmark::op_len++;
	return ret;
}

op_t benchmark::time_op(const char *shortName,const char *desc_)
{
	op_info_t i;
	i.name=shortName; i.desc=desc_; i.isTime=true; i.units="seconds";
	return addOp(i);
}

// benchmark::swap support
static op_t last_op={benchmark::op_null};
static double last_op_start=0;
op_t benchmark::swap(op_t op)
{
	if (op.idx<0 || op.idx>=op_len) {
		osl::bad("benchmark::swap called on invalid operation.\n");
	}
	if (!op_info[op.idx].isTime) {
		osl::bad("benchmark::swap called on non-timing operation.\n");
	}
	double cur=time();
	op_t ret=last_op;
	staticStats.add(cur-last_op_start,ret);
	last_op_start=cur;
	last_op=op;
	return ret;
}


op_t benchmark::count_op(const char *shortName,const char *desc_,const char *units_)
{
	op_info_t i;
	i.name=shortName; i.desc=desc_; i.isTime=false; i.units=units_;
	return addOp(i);
}

/// Print these stats, scaled by scale, where they exceed threshold.
void benchmark::stats::print(FILE *f,const char *what,double scale,double thresh) const
{
	fprintf(f,"%s stats { \n",what);
	for (int op=1;op<op_len;op++) 
		if (t[op]*scale>0) {
			double val=t[op]*scale;
			op_info_t &i=op_info[op];
			const char *units=i.units;
			if (i.isTime) {
				if (val<1.0) {val*=1.0e3; units="ms";}
			}
			fprintf(f,"  %s_%s: %.2f %s\n",what,i.name,val,units);
		}
	fprintf(f,"} \n");
}

