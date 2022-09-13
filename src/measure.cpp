// -*- mode: c++; c-file-style:"stroustrup"; -*-

// Measure: an object to carry measure with its precision.

#include "measure.hpp"

template<typename T, typename S>
std::ostream & operator<<(std::ostream &os, const Measure<T,S>& measure)
{
    std::streamsize saved = std::cout.precision();

    constexpr auto max_digits10 = std::numeric_limits<T>::max_digits10;

    // TODO: document this condition

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



