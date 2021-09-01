
// Monte Carlo simulations to find the Price and Greeks of a simple
// Vanilla Call Option.

#include <cstdlib>
#include <cmath>

#include "badiff.h"
#include "fadiff.h"
#include <iostream>

using namespace std;
using namespace fadbad;

double GetOneGaussianBySummation()
{
	double result=0;
	for (unsigned long j=0; j < 12; j++)
	result += rand()/static_cast<double>(RAND_MAX);
	result -= 6.0;
	return result;
}

double GetOneGaussianByBoxMuller()
{
	double result;
	double x;
	double y;
	double sizeSquared;
	do
	{
		x = 2.0*rand()/static_cast<double>(RAND_MAX)-1;
		y = 2.0*rand()/static_cast<double>(RAND_MAX)-1;
		sizeSquared = x*x + y*y;
	}
	while ( sizeSquared >= 1.0);
	result = x*sqrt(-2*log(sizeSquared)/sizeSquared);
	return result;
}

template <typename ArithT>
ArithT SimpleMonteCarlo(const double Expiry,
	const double Strike,
	const ArithT& Spot,
	const ArithT& Vol,
	const ArithT& r,
	unsigned long NumberOfPaths)
{
	ArithT variance = Vol*Vol*Expiry;
	ArithT rootVariance = sqrt(variance);
	ArithT itoCorrection = -0.5*variance;
	ArithT movedSpot = Spot*exp(r*Expiry +itoCorrection);
	ArithT thisSpot;
	ArithT runningSum=0;
	for (unsigned long i=0; i < NumberOfPaths; i++)
	{
		ArithT thisGaussian = GetOneGaussianByBoxMuller();
		thisSpot = movedSpot*exp( rootVariance*thisGaussian);
		ArithT thisPayoff = thisSpot - Strike;
		thisPayoff = thisPayoff >0 ? thisPayoff : 0;
		runningSum += thisPayoff;
	}
	ArithT mean = runningSum / double(NumberOfPaths);
	mean *= exp(-r*Expiry);
	return mean;
}

template <typename ArithT>
void BADSimpleMonteCarlo(const double Expiry,
	const double Strike,
	const ArithT& Spot,
	const ArithT& Vol,
	const ArithT& r,
	unsigned long NumberOfPaths,
	ArithT& Price,
	ArithT& dPriceSpot,
	ArithT& dPriceVol,
	ArithT& dPriceR)
{
	B< ArithT > BSpot(Spot);
	B< ArithT > BVol(Vol);
	B< ArithT > Br(r);
	B< ArithT > BPrice=SimpleMonteCarlo(Expiry,Strike,BSpot,BVol,Br,NumberOfPaths);
	BPrice.diff(0,1);
	Price=BPrice.x();
	dPriceSpot=BSpot.d(0);
	dPriceVol=BVol.d(0);
	dPriceR=Br.d(0);
}

template <typename ArithT>
void FADSimpleMonteCarlo(const double Expiry,
	const double Strike,
	const ArithT& Spot,
	const ArithT& Vol,
	const ArithT& r,
	unsigned long NumberOfPaths,
	ArithT& Price,
	ArithT& dPriceSpot,
	ArithT& dPriceVol,
	ArithT& dPriceR)
{
	F< ArithT > FSpot(Spot);
	F< ArithT > FVol(Vol);
	F< ArithT > Fr(r);
	FSpot.diff(0,3);
	FVol.diff(1,3);
	Fr.diff(2,3);
	F< ArithT > FPrice=SimpleMonteCarlo(Expiry,Strike,FSpot,FVol,Fr,NumberOfPaths);
	Price=FPrice.x();
	dPriceSpot=FPrice.d(0);
	dPriceVol=FPrice.d(1);
	dPriceR=FPrice.d(2);
}

template <typename ArithT>
void FADFADSimpleMonteCarlo(const double Expiry,
	const double Strike,
	const ArithT& Spot,
	const ArithT& Vol,
	const ArithT& r,
	unsigned long NumberOfPaths,
	ArithT& Price,
	ArithT& dPriceSpot,
	ArithT& dPriceVol,
	ArithT& dPriceR,
	ArithT& dPrice2Spot)
{
	F< ArithT > FSpot(Spot);
	F< ArithT > FVol(Vol);
	F< ArithT > Fr(r);
	FSpot.diff(0,1);
	F< ArithT > FPrice;
	F< ArithT > FdPriceSpot;
	F< ArithT > FdPriceVol;
	F< ArithT > FdPriceR;
	FADSimpleMonteCarlo(Expiry,Strike,FSpot,FVol,Fr,NumberOfPaths,
		FPrice, FdPriceSpot, FdPriceVol, FdPriceR);
	Price=FPrice.x();
	dPriceSpot=FdPriceSpot.x();
	dPriceVol=FdPriceVol.x();
	dPriceR=FdPriceR.x();
	dPrice2Spot=FdPriceSpot.d(0);
}

template <typename ArithT>
void FADBADSimpleMonteCarlo(const double Expiry,
	const double Strike,
	const ArithT& Spot,
	const ArithT& Vol,
	const ArithT& r,
	unsigned long NumberOfPaths,
	ArithT& Price,
	ArithT& dPriceSpot,
	ArithT& dPriceVol,
	ArithT& dPriceR,
	ArithT& dPrice2Spot)
{
	F< ArithT > FSpot(Spot);
	F< ArithT > FVol(Vol);
	F< ArithT > Fr(r);
	FSpot.diff(0,1);
	F< ArithT > FPrice;
	F< ArithT > FdPriceSpot;
	F< ArithT > FdPriceVol;
	F< ArithT > FdPriceR;
	BADSimpleMonteCarlo(Expiry,Strike,FSpot,FVol,Fr,NumberOfPaths,
		FPrice, FdPriceSpot, FdPriceVol, FdPriceR);
	Price=FPrice.x();
	dPriceSpot=FdPriceSpot.x();
	dPriceVol=FdPriceVol.x();
	dPriceR=FdPriceR.x();
	dPrice2Spot=FdPriceSpot.d(0);
}


int main()
{
	double Strike=1.3000;
	double Expiry=1.0;
	double Spot=1.3000;
	double Vol=.08;
	double r=0.06;
	double Price;
	double dPriceSpot;
	double dPriceVol;
	double dPriceR;	
	double dPrice2Spot;

	srand(0);
	
	FADSimpleMonteCarlo(Expiry,Strike,Spot,Vol,r,1000,
		Price,dPriceSpot,dPriceVol,dPriceR);
	
	cout << "Price=" << Price << endl;
	cout << "dPriceSpot=" << dPriceSpot << endl;
	cout << "dPriceVol=" << dPriceVol << endl;
	cout << "dPriceR=" << dPriceR << endl;
	
	srand(0);
	
	BADSimpleMonteCarlo(Expiry,Strike,Spot,Vol,r,1000,
		Price,dPriceSpot,dPriceVol,dPriceR);
	
	cout << "Price=" << Price << endl;
	cout << "dPriceSpot=" << dPriceSpot << endl;
	cout << "dPriceVol=" << dPriceVol << endl;
	cout << "dPriceR=" << dPriceR << endl;

	srand(0);
	
	// NOTICE: that the gamma-value is wrong since the 
	// derivative of the intrinsic value as a function of 
	// spot is not Lipschitz.

	FADFADSimpleMonteCarlo(Expiry,Strike,Spot,Vol,r,1000,
		Price,dPriceSpot,dPriceVol,dPriceR,dPrice2Spot);

	cout << "Price=" << Price << endl;
	cout << "dPriceSpot=" << dPriceSpot << endl;
	cout << "dPriceVol=" << dPriceVol << endl;
	cout << "dPriceR=" << dPriceR << endl;
	cout << "dPrice2Spot=" << dPrice2Spot << endl;

	srand(0);
	
	FADFADSimpleMonteCarlo(Expiry,Strike,Spot,Vol,r,1000,
		Price,dPriceSpot,dPriceVol,dPriceR,dPrice2Spot);

	cout << "Price=" << Price << endl;
	cout << "dPriceSpot=" << dPriceSpot << endl;
	cout << "dPriceVol=" << dPriceVol << endl;
	cout << "dPriceR=" << dPriceR << endl;
	cout << "dPrice2Spot=" << dPrice2Spot << endl;

	srand(0);

	FADBADSimpleMonteCarlo(Expiry,Strike,Spot,Vol,r,1000,
		Price,dPriceSpot,dPriceVol,dPriceR,dPrice2Spot);

	cout << "Price=" << Price << endl;
	cout << "dPriceSpot=" << dPriceSpot << endl;
	cout << "dPriceVol=" << dPriceVol << endl;
	cout << "dPriceR=" << dPriceR << endl;
	cout << "dPrice2Spot=" << dPrice2Spot << endl;

	return 0;
}
