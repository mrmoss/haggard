/*
Physical units handling code.

Orion Sky Lawlor, olawlor@acm.org, 2003/12/6
*/
#ifndef __OSL_UNITS_H
#define __OSL_UNITS_H

namespace osl {

/// Meters per centimeter (exact)
const double metersFmCm=1.0/100.0;

/// Centimeters per US inch (exact)
const double cmFmInch=2.54;

/// Meters per US inch (exact)
const double metersFmInch=metersFmCm*cmFmInch;

/// Inches per US feet (exact)
const double inchFmFeet=12.0;

/// Meters per US feet (exact)
const double metersFmFeet=metersFmInch*inchFmFeet;

/// Speed of light in meters per second (exact)
const double metersFmSeconds = 299792458.0; 

/// Zero of the Celsius scale in Kelvin (exact)
const double celciusFmKelvin = 273.15;

/// Standard atmosphere atm in Pascals (exact)
const double atmFmPa = 101325.0;
};

/// Mathematical and physical constants:
namespace constants {
	/// Ratio of circumference to diameter [pure]
        const double pi=3.14159265358979323;
	
	/// Speed of light in vacuum [ meters/second ]
        const double c=osl::metersFmSeconds;
	
	/// Planck's constant [ Joule-seconds ]
        const double h=6.6260687652e-34;
	/// Planck's constant, barred (Dirac's constant)
        const double hBar=h/(2*pi);
	
	/// Boltzmann constant [ Joule / Kelvin ]
        const double boltzmann=1.380650324e-23;
	/// Avogadro's number [ atoms per mole ]
	const double avagadro=6.0221419947e+23;
	
	/// Stefan-Boltzmann constant [ Watts / (meter^2 K^4) ]
	const double stefanBoltzmann=hBar*c/boltzmann;
	
	/// Gravitational constant [ newton meter^2 / kilogram^2 ]
	const double g=6.6725985e-11;
	
};

#endif
