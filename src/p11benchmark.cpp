// p11benchmark.cpp : a base class for implementing performance test cases

#include <iostream>
#include <boost/timer/timer.hpp>
#include "p11benchmark.hpp"

P11Benchmark::P11Benchmark(Session &session, const std::string &name, const std::string &label, ObjectClass objectclass)
    : m_session(session), m_name(name), m_label(label), m_objectclass(objectclass)
{
    std::cout << "Checking speed for "<< name << std::endl
	      << "========================================================================" << std::endl;
}


void P11Benchmark::execute( std::vector<uint8_t> &payload, unsigned long iterations)
{

    try {
	m_payload = payload;	// remember the payload

	AttributeContainer search_template;
	search_template.add_string( AttributeType::Label, m_label );
	search_template.add_class( m_objectclass );

	auto found_objs = Object::search<Object>( m_session, search_template.attributes() );

	if( found_objs.size()==0 ) {
	    std::cerr << "Error: no object found for label '" << m_label << "'" << std::endl;
	} else {
	    for ( auto &obj: found_objs) {

		prepare(obj);

		std::cout << iterations << " operations with '" << m_label << "' on a buffer of " << m_payload.size() << " bytes" << std::endl;
		{
		    boost::timer::cpu_timer t;
		    for (unsigned long x=0; x<iterations; x++) {
			crashtestdummy();
		    }
		    boost::timer::cpu_times const elapsed( t.elapsed() );
		    // elapsed is in ns. We want to find a rate per second.

		    std::cout << "Iterations  : " << iterations << std::endl
			      << "Elapsed (ms): " << elapsed.wall/1000000LL << std::endl
			      << "TPS         : " << iterations / float(elapsed.wall/1000000LL) * 1000 << std::endl;
		}
	    }
	}
    } catch (Botan::Exception &bexc) {
	std::cerr << "ERROR:: caught an exception:" << bexc.what() << std::endl;
	// we print the exception, and go on with next tests.
    }
}

