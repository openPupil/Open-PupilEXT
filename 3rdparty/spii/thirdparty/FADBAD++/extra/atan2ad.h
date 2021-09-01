// Copyright (C) 1996-2007 Ole Stauning & Claus Bendtsen (fadbad@uning.dk)
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
// COPYRIGHT NOTICE
// ***************************************************************

// atan2 FADBAD++ formulas

#include "fadiff.h"

namespace fadbad
{
	template <typename T> struct ATan2Op
	{
		template <typename X, typename Y>
                static T atan2(const X& x, const Y& y) { return ::atan2(x,y); }
	};
	
	// Forward automatic differentiation of atan2:
	template <typename T, unsigned int N>
	inline FTypeName<T,N> atan2(const FTypeName<T,N>& y, const FTypeName<T,N>& x)
	{
		FTypeName<T,N> c(ATan2Op<T>::atan2(y.val(),x.val()));
		if (!y.depend() && !x.depend()) return c;
	
		T tmp(Op<T>::mySqr(y.val())+Op<T>::mySqr(x.val()));
		c.setDepend(y,x);
		for(unsigned int i=0;i<c.size();++i) c[i]=(x.val()*y[i]-y.val()*x[i])/tmp;
		return c;
	}

	// TODO: write backward- and taylor methods.


	template <typename U, unsigned int N> struct ATan2Op< FTypeName<U,N> >
	{
		typedef FTypeName<U,N> T;
		template <typename X, typename Y>
		static T atan2(const X& x, const Y& y) { return fadbad::atan2(x,y); }
	};
}


/* TEST:
#include <iostream>
int main()
{
	fadbad::F<double,2> x(.37754),y(.9364),z;
	x.diff(0);y.diff(1);
	z=atan2(y,x);
	std::cout<<"z="<<z.val()<<" z0="<<z[0]<<" z1="<<z[1]<<std::endl;
	z=atan(y/x);
	std::cout<<"z="<<z.val()<<" z0="<<z[0]<<" z1="<<z[1]<<std::endl;
	return 0;
}
*/

