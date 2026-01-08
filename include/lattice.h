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
 * @brief Applies deep-insertion to lattice basis
 *
 * @param k index
 * @param l index
 */
void DeepInsertion(const int k, const int l);

void UpdateDeepInsertionGSO(const long i, const long k, const long end);

/**
 * @brief Computes volume of lattice
 *
 * @return NTL::ZZ
 */
NTL::ZZ Volume();

/**
 * @brief Computes GSA-slope
 *
 * @return NTL::RR GSA-slope
 */
NTL::RR ComputeSlope();

/**
 * @brief Computes RHF
 *
 * @return NTL::RR RHF
 */
NTL::RR ComputeRHF();

/**
 * @brief 
 * 
 * @param start 
 * @param end 
 * @param v 
 * @return true 
 * @return false 
 */
bool ENUM(const int start, const int end, NTL::vec_ZZ &v);

#endif // !LATTICE_H
