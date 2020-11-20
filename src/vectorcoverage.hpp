// vectorcoverage.hpp: a class to select vector sizes to use during testing
// -*- mode: c++; c-file-style:"stroustrup"; -*-

#if !defined(VECTORCOVERAGE_H)
#define VECTORCOVERAGE_H

#include <cstdint>

template <typename T>
class SetWrapper
{

public:

    SetWrapper(std::string tocover);

    bool contains(std::string vsize);
    bool contains(T vsize);

    // https://stackoverflow.com/a/46432381/979318
    // iterator boilerplate, we support range iterator only
    //
    using set_type = std::set<T>;
    using iterator = typename set_type::iterator;
    using const_iterator = typename set_type::const_iterator;

    inline iterator begin() noexcept { return m_vector_coverage.begin(); }
    inline const_iterator cbegin() const noexcept { return m_vector_coverage.cbegin(); }
    inline iterator end() noexcept { return m_vector_coverage.end(); }
    inline const_iterator cend() const noexcept { return m_vector_coverage.cend(); }

private:
    set_type m_vector_coverage;

};

using VectorCoverage = SetWrapper<std::uint32_t>;



#endif // VECTORCOVERAGE_H
