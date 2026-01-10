#include "core.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <regex>

#include <windows.h>
#include <commctrl.h>

#include <NTL/ZZ.h>
#include <NTL/vec_ZZ.h>

#include "tools.h"
#include "lattice.h"

REDUCE reduce = REDUCE::NONE;

void InitLattice()
{
    lattice.rank = 0L;
}

std::string ZZToString(const NTL::ZZ &z)
{
    std::stringstream buffer;
    buffer << z;
    return buffer.str();
}

std::string RRToString(const NTL::RR &r)
{
    std::stringstream buffer;
    buffer << r;
    return buffer.str();
}

std::string vec_ZZToString(const NTL::vec_ZZ &x)
{
    std::stringstream buffer;
    buffer << x;
    return buffer.str();
}

std::string mat_ZZToString(const NTL::mat_ZZ &b)
{
    std::stringstream buffer;
    buffer << b;
    return std::regex_replace(buffer.str(), std::regex("\n"), "\r\n");
}

NTL::RR Dot(const NTL::vec_ZZ a, const NTL::vec_ZZ b)
{
    NTL::ZZ t;
    NTL::InnerProduct(t, a, b);
    return NTL::to_RR(t);
}

void Generator(const int rank, const int seed)
{
    const long bit = 10L;

    NTL::vec_ZZ v;
    generate_random_HNF(v, rank, bit, NTL::to_ZZ(seed));
    lattice.basis.SetDims(rank, rank);
    lattice.rank = rank;
    NTL::clear(lattice.basis);
    lattice.basis(1, 1) = v(1);
    for (int i = 2; i <= lattice.rank; i++)
    {
        lattice.basis(i, 1) = v(i);
        lattice.basis(i, i) = 1;
    }
}

void PrintRef(HWND hWnd)
{
    int i = 1;
    std::wstring ref = L"";
    std::vector<std::wstring> refs;
    refs.emplace_back(L"A. K. Lenstra, H. W. Lenstra, and L. Lovász. Factoring polynomials with rational coefficients. 1982\r\n");
    refs.emplace_back(L"C. P. Schnorr and M. Euchner. Lattice basis reduction: Improved practical algorithms and solving subset sum problems. 1994\r\n");
    refs.emplace_back(L"P. Q. Nguyen and D. Stehlé. Floating-point LLL revisited. 2004\r\n");
    refs.emplace_back(L"P. Q. Nguyen and D. Stehlé. An LLL algorithm with quadratic complexity. 2009\r\n");
    refs.emplace_back(L"N. Gama, P. Q. Nguyen, and, O. Regev. Lattice enumeration using extreme pruning. 2010\r\n");
    refs.emplace_back(L"M. R. Bremner. Lattice Basis Reduction: An Introduction to the LLL Algorithm and Its Applications. 2011\r\n");
    refs.emplace_back(L"F. Fontein, M. Schneider, and U. Wagner. PotLLL: A polynomial time version of LLL with deep insertions. 2014\r\n");
    refs.emplace_back(L"J. Yamaguchi and M. Yasuda. Explicit formula for Gram-Schmidt vectors in LLL with deep insertions and its applications. 2017\r\n");
    refs.emplace_back(L"R. Bottesch, M. W. Haslbeck, and R. Thiemann. A verified efficient implementation of the LLL basis reduction algorithm. 2018\r\n");
    refs.emplace_back(L"安田 雅哉, 山口 純平, 大岡 美智子, and 中邑 聡史. 双対版DeepBKZ基底簡約の開発とLWEチャレンジ解読への適用. 2018\r\n");
    refs.emplace_back(L"青野 良範 and 安田 雅哉. 格子暗号解読のための数学的基礎：格子基底簡約アルゴリズム入門. 2019\r\n");
    refs.emplace_back(L"L. Ducas, L. N. Pulles, and M. Stevens. Towards a modern LLL implementation. 2025\r\n");
    refs.emplace_back(L"佐藤 新 and 安田 雅哉. 自己双対型PotBKZ基底簡約の提案とBKZとの比較. 2025\r\n");
    refs.emplace_back(L"V. Shoup. NTL: A Library for doing Number Theory. http://www.shoup.net/ntl/\r\n");
    refs.emplace_back(L"TU Darmstadt. SVP Challenge. https://www.latticechallenge.org/svp-challenge/\r\n");
    for (std::wstring s : refs)
    {
        ref += L"[" + std::to_wstring(i) + L"] " + s;
        ++i;
    }

    MessageBoxW(hWnd, ref.c_str(), L"Info", MB_OK);
}
