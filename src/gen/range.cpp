#include "halcheck/gen/range.hpp" // IWYU pragma: keep

using namespace halcheck;

template unsigned char gen::range_t::operator()(lib::atom, unsigned char, unsigned char) const;
template signed char gen::range_t::operator()(lib::atom, signed char, signed char) const;
template char gen::range_t::operator()(lib::atom, char, char) const;
template unsigned short gen::range_t::operator()(lib::atom, unsigned short, unsigned short) const;
template signed short gen::range_t::operator()(lib::atom, signed short, signed short) const;
template unsigned int gen::range_t::operator()(lib::atom, unsigned int, unsigned int) const;
template signed int gen::range_t::operator()(lib::atom, signed int, signed int) const;
template unsigned long gen::range_t::operator()(lib::atom, unsigned long, unsigned long) const;
template signed long gen::range_t::operator()(lib::atom, signed long, signed long) const;
template unsigned long long gen::range_t::operator()(lib::atom, unsigned long long, unsigned long long) const;
template signed long long gen::range_t::operator()(lib::atom, signed long long, signed long long) const;
template float gen::range_t::operator()(lib::atom, float, float) const;
template double gen::range_t::operator()(lib::atom, double, double) const;
template long double gen::range_t::operator()(lib::atom, long double, double) const;
