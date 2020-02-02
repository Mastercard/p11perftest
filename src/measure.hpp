// -*- mode: c++; c-file-style:"stroustrup"; -*-

// Measure: an object to carry measure with its precision.

#if !defined(MEASURE_HPP)
#define MEASURE_HPP

#include <iostream>
#include <ctgmath>
#include <limits>
#include <ios>
#include <iomanip>
#include "../config.h"

// forward declaration for friend function
template<typename T, typename S>
class Measure;

template<typename T, typename S>
std::ostream & operator<<(std::ostream &os, const Measure<T,S>& measure);


// declaration of Measure class
template<typename T=double, typename S=std::string>
class Measure {
    T m_value;			// the raw value of the measure
    T m_error;			// the raw value of the error on measure
    S m_unit;			// unit of the measure (character string)
    int m_value_order;		// the decimal order of the measure
    int m_error_order;		// the decimal order of the error on measure
    int m_precision;		// the precision (in decimal digits) wanted on measure
    int m_error_precision;	// the precision (in decimal digits) given on measure error

public:
    Measure(T val, T err, S unit, int e_precision=2) :
	m_value(val),
	m_error(err),
	m_unit(unit),
	m_error_precision(e_precision),
	m_value_order(ceil(log10(val))),
	m_error_order(ceil(log10(err))) {

	auto digits = [](T n) -> int { return ceil(abs(log10(n))) * copysign(1,log10(n)); };

	m_precision = digits(val) - digits(err) + 1;
    }

    inline const T value() { return rounder(m_value, m_precision); }
    inline const T error() { return rounder(m_error, m_error_precision); }
    inline const T relerr() { return fabs(error()/value()); }
    inline std::pair<T,T> value_error() { return std::make_pair(value(),error()); }
    inline const S unit() { return m_unit; }

private:
    // to print a Measure instance
    friend std::ostream & operator<<<T>(std::ostream &os, const Measure<T>& f);

    // rounder() is used to chop figures from n, given a wanted precision (p)
    T rounder(T n, T p) const {
        auto shift = p - ceil(log10(n));
	return round(n*pow(10,shift)) / pow(10,shift);
    }
};

template<typename T, typename S>
std::ostream & operator<<(std::ostream &os, const Measure<T,S>& measure)
{
    std::streamsize saved = std::cout.precision();

    constexpr auto max_digits10 = std::numeric_limits<T>::max_digits10;

    bool print_fixed = measure.m_value_order < max_digits10 &&
                       measure.m_error_order < max_digits10 &&
		       measure.m_value_order - measure.m_error_order < max_digits10 &&
		       measure.m_value_order > 0;
    if(print_fixed==true) {
	os << std::fixed << std::setprecision(measure.m_error_order<0 ? measure.m_precision - measure.m_value_order : 0);
    } else {
	os << std::setprecision(measure.m_precision);
    }

    os << measure.rounder(measure.m_value, measure.m_precision);

    if(print_fixed==true) {
	os << std::defaultfloat;
    }

    os << ' ' << measure.m_unit
       << " +/- "
       << std::setprecision(measure.m_error_precision)
       << measure.rounder(measure.m_error, measure.m_error_precision)
       << std::setprecision(saved);

    return os;
}



#endif // MEASURE_HPP

