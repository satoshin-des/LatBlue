#ifndef LATTICE_H
#define LATTICE_H

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

/**
 * @brief Computes GSO informations
 *
 */
void ComputeGSO();

/**
 * @brief Computes GSA-slope
 *
 * @return NTL::RR GSA-slope
 */
NTL::RR ComputeSlope();

#endif // !LATTICE_H
