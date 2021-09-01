// Copyright (C) 1996-2007 Ole Stauning (fadbad@uning.dk)
// All rights reserved.

// This code is provided "as is", without any warranty of any kind,
// either expressed or implied, including but not limited to, any implied
// warranty of merchantibility or fitness for any purpose. In no event
// will any party who distributed the code be liable for damages or for
// any claim(s) by any other party, including but not limited to, any
// lost profits, lost monies, lost data or data rendered inaccurate,
// losses sustained by third parties, or any other special, incidental or
// consequential damages arising out of the use or inability to use the
// program, even if the possibility of such damages has been advised
// against. The entire risk as to the quality, the performance, and the
// fitness of the program for any particular purpose lies with the party
// using the code.

// This code, and any derivative of this code, may not be used in a
// commercial package without the prior explicit written permission of
// the authors. Verbatim copies of this code may be made and distributed
// in any medium, provided that this copyright notice is not removed or
// altered in any way. No fees may be charged for distribution of the
// codes, other than a fee to cover the cost of the media and a
// reasonable handling fee.

// ***************************************************************
// ANY USE OF THIS CODE CONSTITUTES ACCEPTANCE OF THE TERMS OF THE
//                         COPYRIGHT NOTICE
// ***************************************************************

//
// NORMAL DISTRIBUTION FUNCTIONS AND AUTOMATIC DIFFERENTIATION OF THESE

#ifndef _NDF_H
#define _NDF_H

#include <limits>

namespace fadbad
{

// The normal distribution
template <class T>
inline T nd(const T& x)
{
	static const double sqrt2pi=Op<double>::mySqrt(Op<double>::myTwo()*Op<double>::myPI());
	return Op<T>::myExp(-0.5*Op<T>::mySqr(x))/sqrt2pi;
}


// The cumulative normal distribution (approximation)
// NOTE: Don't try to apply AD on an approximation -
// this will cause inexact results!
inline double cnd(const double x)
{
	// CND(X)=0 for X<-37 and CND(X)=1 for X>37 when
	// computing in double precision:
	static const double CNDMAX=37;
	
	double cnorm;
	double xabs=fabs(x);
	if (xabs>CNDMAX)
	{
		cnorm=0.0;
	}
	else
	{
		double build;
		double e=Op<double>::myExp(-0.5*Op<double>::mySqr(xabs));
		if (xabs<7.07106781186547)
		{
			build = 3.52624965998911E-02 * xabs + 0.700383064443688;
			build = build * xabs + 6.37396220353165;
			build = build * xabs + 33.912866078383;
			build = build * xabs + 112.079291497871;
			build = build * xabs + 221.213596169931;
			build = build * xabs + 220.206867912376;
			cnorm = e * build;
			build = 8.83883476483184E-02 * xabs + 1.75566716318264;
			build = build * xabs + 16.064177579207;
			build = build * xabs + 86.7807322029461;
			build = build * xabs + 296.564248779674;
			build = build * xabs + 637.333633378831;
			build = build * xabs + 793.826512519948;
			build = build * xabs + 440.413735824752;
			cnorm /= build;
		}
		else
		{
			build = xabs + 0.65;
			build = xabs + 4.0 / build;
			build = xabs + 3.0 / build;
			build = xabs + 2.0 / build;
			build = xabs + 1.0 / build;
			cnorm = e / build / 2.506628274631;
		}
	}
	if (x>0) cnorm=1-cnorm;
	return cnorm;
}

// The inverse cumulative normal distribution (approximation)
// NOTE: Don't try to apply AD on an approximation -
// this will cause inexact results!
inline double icnd(const double p)
{
	static const double a[6] = 
	{
		-3.969683028665376e+01,  2.209460984245205e+02,
		-2.759285104469687e+02,  1.383577518672690e+02,
		-3.066479806614716e+01,  2.506628277459239e+00
	};
	static const double b[5] = 
	{
		-5.447609879822406e+01,  1.615858368580409e+02,
		-1.556989798598866e+02,  6.680131188771972e+01,
		-1.328068155288572e+01
	};
	static const double c[6] = 
	{
		-7.784894002430293e-03, -3.223964580411365e-01,
		-2.400758277161838e+00, -2.549732539343734e+00,
		4.374664141464968e+00,  2.938163982698783e+00
	};
	static const double d[4] = 
	{
		7.784695709041462e-03,  3.224671290700398e-01,
		2.445134137142996e+00,  3.754408661907416e+00
	};
	static const double sqrt2pi=Op<double>::mySqrt(Op<double>::myTwo()*Op<double>::myPI());
	
	double q, t, u;
	
	if (p > 1.0 || p < 0.0)
		return std::numeric_limits<double>::quiet_NaN();
	if (p == 0.0)
		return -std::numeric_limits<double>::infinity();
	if (p == 1.0)
		return std::numeric_limits<double>::infinity();
	q = p<.5?p:1-p;
	if (q > 0.02425) 
	{
		// Rational approximation for central region.
		u = q-0.5;
		t = u*u;
		u = u*(((((a[0]*t+a[1])*t+a[2])*t+a[3])*t+a[4])*t+a[5])
		/(((((b[0]*t+b[1])*t+b[2])*t+b[3])*t+b[4])*t+1);
	}
	else 
	{
		// Rational approximation for tail region.
		t = Op<double>::mySqrt(Op<double>::myNeg(Op<double>::myTwo()*Op<double>::myLog(q)));
		u = (((((c[0]*t+c[1])*t+c[2])*t+c[3])*t+c[4])*t+c[5])
		/((((d[0]*t+d[1])*t+d[2])*t+d[3])*t+1);
	}
	// The relative error of the approximation has absolute value less
	// than 1.15e-9.  One iteration of Halley's rational method (third
	// order) gives full machine precision... 
	t = cnd(u)-q;               // error
	t = t*sqrt2pi*Op<double>::myExp(Op<double>::mySqr(u)/Op<double>::myTwo());   // f(u)/df(u)
	u = u-t/(1.0+u*t/Op<double>::myTwo());          // Halley's method
	
	return (p > 0.5 ? -u : u);
};

} // namespace fadbad

#endif

