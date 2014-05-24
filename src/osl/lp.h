/*
Orion's Standard Library
Orion Sky Lawlor, 10/11/2002
NAME:		osl/lp.h

DESCRIPTION:	C++ Linear Optimization library

More advanced color classes.
*/
#ifndef __OSL_LP_H
#define __OSL_LP_H

namespace osl {

/**
 * Describes a linear optimization problem.
 */
class lp {
	void *sto;
public:
	/**
	 * Create a linear optimization problem with nVar 
	 * unknowns.  The objective function is equal to 
	 *    f(v) = sum_i opt[i]*v[i]
	 * dir controls whether to minimize (the default) 
	 *   or maximize the objective function.
	 * nConst is the number of constraints, if known.
	 */
	enum opt_t {
	opt_max=10, //A maximization problem
	opt_min=11, //A minimization problem
	};
	lp(int nVar,const double *opt,opt_t dir=opt_min,int nConst=0);
	~lp();
	
	/**
	 * Add another constraint to this problem.  The problem
	 * will only accept unknown values that satisfy
	 *    sum_i con[i]*v[i] op val
	 */
	enum constraint_t {
	constraint_le=0, //A less-than-or-equal constraint
	constraint_eq=1, //An equality constraint
	constraint_ge=2  //A greater-than-or-equal constraint
	};
	void addConstraint(const double *con,constraint_t constraint,double val);
	
	///Reset the upper bound of variable i to val (default INF)
	void setUpperBound(int i,double val);
	///Reset the lower bound of variable i to val (default 0)
	void setLowerBound(int i,double val);
	
	/**
	 * Solve, and return the values of the optimal unknowns
	 * into vals, which must have nVar entries.
	 * Returns false if the problem could not be solved.
	 */
	bool solve(double *vals);
};


};

#endif


