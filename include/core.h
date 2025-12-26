#ifndef CORE_H
#define CORE_H

#include <iostream>

#include <NTL/mat_ZZ.h>

extern NTL::mat_ZZ B;
extern int n;

/**
 * @brief Generates Goldstein-Mayer lattice
 *
 * @param seed seed for random-generation
 */
void generator(const int seed);

#endif // !CORE_H
