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


std::unique_ptr<ptree> P11Benchmark::execute( std::vector<uint8_t> &payload, unsigned long iterations)
{

    std::unique_ptr<ptree> rv(new ptree);

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
		    boost::timer::cpu_times elapsed;
		    boost::timer::cpu_times started;

		    elapsed.clear();
		    started.clear();

		    for (unsigned long x=0; x<iterations; x++) {
			t.start(); // start it
			started.wall = t.elapsed().wall; // remember wall clock
			crashtestdummy();
			elapsed.wall += t.elapsed().wall - started.wall;
		    }

		    std::cout << "Iterations : " << iterations << std::endl
			      << "elapsed ms : " << elapsed.wall/1000000.0 << std::endl
			      << "latency ms : " << elapsed.wall/1000000.0 / float(iterations) << std::endl
			      << "TPS        : " << iterations / float(elapsed.wall/1000000.0) * 1000 << std::endl;

		    rv->add("iterations", iterations);
		    rv->add("elapsed", elapsed.wall/1000000LL);
		    rv->add("latency", elapsed.wall/1000000.0 / float(iterations));
		    rv->add("tps", iterations / float(elapsed.wall/1000000LL) * 1000);
		}
	    }
	}
    } catch (Botan::Exception &bexc) {
	std::cerr << "ERROR:: caught an exception:" << bexc.what() << std::endl;
	// we print the exception, and go on with next tests.
    }
    
    return rv;
}
