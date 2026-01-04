#ifndef REDUCTION_H
#define REDUCTION_H

#include <windows.h>

#include <NTL/mat_RR.h>
#include <NTL/vec_RR.h>

void SizeReduce(const int i, const int j);

void SizeReduceL2(const double eta, const int k, NTL::mat_RR &r, NTL::vec_RR &s);

/**
 * @brief Applies LLL-reduction for lattice basis with L2 algorithm
 *
 * @param delta reduction parameter
 */
void L2Reduce(HWND hWnd, UINT Msg, const double delta);

/**
 * @brief Applies LLL-reduction for lattice basis
 *
 * @param delta reduction parameter
 * @param end
 * @param h
 */
void LLLReduce(const double delta, const int end, const int h);

void DeepLLLReduce(HWND hWnd, UINT Msg, const double delta, const int end, const int h);

#endif // !REDUCTION_H
