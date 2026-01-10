#ifndef REDUCTION_H
#define REDUCTION_H

#include <windows.h>

#include <NTL/mat_RR.h>
#include <NTL/vec_RR.h>

extern double delta; // reduction parameter
extern int gamma;    // reduction parameter for deep-insertion
extern int beta;     // block-size
extern int max_loop; // max tour loops for BKZ

void SizeReduce(const int i, const int j);

void SizeReduceL2(const double eta, const int k, NTL::mat_RR &r, NTL::vec_RR &s);

/**
 * @brief Applies LLL-reduction for lattice basis with L2 algorithm
 *
 */
void L2Reduce(HWND hWnd, UINT Msg);

/**
 * @brief Applies LLL-reduction for lattice basis
 *
 * @param end
 * @param h
 */
void LLLReduce(const int end, const int h);

/**
 * @brief Applies DeepLLL-reduction for lattice basis
 *
 * @param hWnd
 * @param Msg
 * @param end
 * @param h
 */
void DeepLLLReduce(HWND hWnd, UINT Msg, const int end, const int h);

void DeepLLLReduce(const int end, const int h);

/**
 * @brief Applies PotLLL-reduction for lattice basis
 *
 * @param hWnd
 * @param Msg
 * @param end
 * @param h
 */
void PotLLLReduce(HWND hWnd, UINT Msg, const int end, const int h);

/**
 * @brief Applies PotLLL-reduction
 *
 */
void PotLLLReduce();

/**
 * @brief Applies BKZ-reduction for lattice basis
 *
 * @param hWnd
 * @param Msg
 */
void BKZReduce(HWND hWnd, UINT Msg);

void DeepBKZReduce(HWND hWnd, UINT Msg);

/**
 * @brief Applies PotBKZ-reduction for lattice basis
 *
 * @param hWnd
 * @param Msg
 */
void PotBKZReduce(HWND hWnd, UINT Msg);

#endif // !REDUCTION_H
