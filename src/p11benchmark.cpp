// p11benchmark.cpp : a base class for implementing performance test cases

#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "p11benchmark.hpp"
#include "errorcodes.hpp"

// thread sync objects
extern std::mutex greenlight_mtx;
extern std::condition_variable greenlight_cond;
extern bool greenlight;

static std::mutex display_mtx;


P11Benchmark::P11Benchmark(const std::string &name, const std::string &label, ObjectClass objectclass)
    : m_name(name), m_label(label), m_objectclass(objectclass)
{ }

P11Benchmark::P11Benchmark(const P11Benchmark& other)
    : m_name(other.m_name), m_label(other.m_label), m_objectclass(other.m_objectclass)
{
    // std::cout << "copy constructor invoked for " << m_name << std::endl;
}

P11Benchmark& P11Benchmark::operator=(const P11Benchmark& other)
{
    // std::cout << "copy assignment invoked for " << m_name << std::endl;
    m_name = other.m_name;
    m_label = other.m_label;
    m_objectclass = other.m_objectclass;
    return *this;
}

std::string P11Benchmark::features() const
{
    return "Undefined";		// TODO: for each algorithm, retrieve key size and features, to print out.
}


std::pair<nanosecond_type,int> P11Benchmark::execute(Session *session, const std::vector<uint8_t> &payload, unsigned long iterations)
{
    boost::timer::cpu_times elapsed { 0	};
    int return_code = CKR_OK;

    try {
	m_payload = payload;	// remember the payload

	AttributeContainer search_template;
	search_template.add_string( AttributeType::Label, m_label );
	search_template.add_class( m_objectclass );

	auto found_objs = Object::search<Object>( *session, search_template.attributes() );

	if( found_objs.size()==0 ) {
	    std::cerr << "Error: no object found for label '" << m_label << "'" << std::endl;
	} else {
	    for ( auto &obj: found_objs) {

		prepare(*session, obj);

		boost::timer::cpu_timer t;
		boost::timer::cpu_times started;

		elapsed.clear();
		started.clear();

		// wait for green light - all threads are starting together
		{
		    std::unique_lock<std::mutex> greenlight_lck(greenlight_mtx);
		    greenlight_cond.wait(greenlight_lck,[]{ return greenlight; });
		}

		// ok go now!
		for (unsigned long x=0; x<iterations; x++) {
		    t.start(); // start it
		    started.wall = t.elapsed().wall; // remember wall clock
		    crashtestdummy(*session);
		    t.stop(); // stop it
		    elapsed.wall += t.elapsed().wall - started.wall;
		}
	    }
	}
    } catch (Botan::PKCS11::PKCS11_ReturnError &bexc) {
	{
	    std::lock_guard<std::mutex> lg{display_mtx};
	    std::cerr << "ERROR:: " << bexc.what()
		      << " (" << errorcode(bexc.error_code()) << ")" << std::endl;
	}
	return_code = bexc.error_code();
	// we print the exception, and move on
    } catch (...) {
	{
	    std::lock_guard<std::mutex> lg{display_mtx};
	    std::cerr << "ERROR: caught an unmanaged exception" << std::endl;
	}
	// bailing out
	throw;
    }

    return std::pair<nanosecond_type,int> {elapsed.wall, return_code };
}
