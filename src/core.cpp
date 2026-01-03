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
