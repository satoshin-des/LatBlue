#ifndef CORE_H
#define CORE_H

#include <iostream>
#include <string>

#include <NTL/ZZ.h>
#include <NTL/RR.h>
#include <NTL/vec_ZZ.h>

/**
 * @brief enum for lattice reduction algorithm
 *
 */
enum REDUCE
{
    NONE,
    LLL,
    DEEP_LLL,
    POT_LLL,
    BKZ
};

extern REDUCE reduce; // Reduction algorithm

/**
 * @brief Initializes lattice informations
 *
 */
void InitLattice();

NTL::RR Dot(const NTL::vec_ZZ a, const NTL::vec_ZZ b);

/**
 * @brief Converts NTL::ZZ to std::string
 *
 * @param z Integer to convert
 * @return std::string Converted string
 */
std::string ZZToString(const NTL::ZZ &z);

/**
 * @brief Generates Goldstein-Mayer lattice
 *
 * @param seed seed for random-generation
 */
void Generator(const int rank, const int seed);

#endif // !CORE_H
