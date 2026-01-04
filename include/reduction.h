#ifndef REDUCTION_H
#define REDUCTION_H

#include <windows.h>

void SizeReduce(const int i, const int j);

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