#include "core.h"

#include <iostream>
#include <string>
#include <sstream>

#include <NTL/ZZ.h>
#include <NTL/vec_ZZ.h>

#include "tools.h"
#include "lattice.h"

REDUCE reduce = REDUCE::NONE;

void InitLattice()
{
    lattice.rank = 0L;
}

std::string ZZToString(const NTL::ZZ &z)
{
    std::stringstream buffer;
    buffer << z;
    return buffer.str();
}

std::string RRToString(const NTL::RR &r)
{
    std::stringstream buffer;
    buffer << r;
    return buffer.str();
}

NTL::RR Dot(const NTL::vec_ZZ a, const NTL::vec_ZZ b)
{
    NTL::ZZ t;
    NTL::InnerProduct(t, a, b);
    return NTL::to_RR(t);
}

void Generator(const int rank, const int seed)
{
    const long bit = 10L;

    NTL::vec_ZZ v;
    generate_random_HNF(v, rank, bit, NTL::to_ZZ(seed));
    lattice.basis.SetDims(rank, rank);
    lattice.rank = rank;
    NTL::clear(lattice.basis);
    lattice.basis(1, 1) = v(1);
    for (int i = 2; i <= lattice.rank; i++)
    {
        lattice.basis(i, 1) = v(i);
        lattice.basis(i, i) = 1;
    }
}
