#ifndef CORE_H
#define CORE_H

#include <iostream>

/**
 * @brief Initializes lattice informations
 *
 */
void InitLattice();

/**
 * @brief Generates Goldstein-Mayer lattice
 *
 * @param seed seed for random-generation
 */
void Generator(const int rank, const int seed);

#endif // !CORE_H
