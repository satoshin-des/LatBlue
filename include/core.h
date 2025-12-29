#ifndef CORE_H
#define CORE_H

#include <iostream>
#include <string>

#include <NTL/ZZ.h>

/**
 * @brief Initializes lattice informations
 *
 */
void InitLattice();

std::string ZZToString(const NTL::ZZ &z);

/**
 * @brief Generates Goldstein-Mayer lattice
 *
 * @param seed seed for random-generation
 */
void Generator(const int rank, const int seed);

#endif // !CORE_H
