#ifndef CORE_H
#define CORE_H

#include <iostream>

#include <NTL/mat_ZZ.h>
#include <NTL/vec_RR.h>
#include <NTL/mat_RR.h>

struct Lattice
{
    long rank;
    NTL::mat_ZZ basis;
    NTL::vec_RR B;
    NTL::mat_RR mu;
};

extern Lattice lattice;

void initLattice();

/**
 * @brief Generates Goldstein-Mayer lattice
 *
 * @param seed seed for random-generation
 */
void generator(const int rank, const int seed);

#endif // !CORE_H
