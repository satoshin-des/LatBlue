#include "core.h"

#include <iostream>
#include <sstream>

#include <NTL/ZZ.h>

#include "tools.h"

Lattice lattice;

void initLattice()
{
    lattice.rank = 0L;
}

void generator(const int rank, const int seed)
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
