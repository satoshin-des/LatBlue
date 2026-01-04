#include "reduction.h"

#include <NTL/ZZ.h>
#include <NTL/RR.h>
#include <NTL/vec_ZZ.h>
#include <NTL/mat_RR.h>
#include <NTL/vec_RR.h>
#include <NTL/LLL.h>

#include "core.h"
#include "lattice.h"

void SizeReduce(const int i, const int j)
{
    if ((lattice.mu[i][j] > 0.5) || (lattice.mu[i][j] < -0.5))
    {
        long k;
        const NTL::ZZ q = NTL::RoundToZZ(lattice.mu[i][j]);

        lattice.basis[i] -= q * lattice.basis[j];
        for (k = 0; k <= j; ++k)
        {
            lattice.mu[i][k] -= lattice.mu[j][k] * NTL::to_RR(q);
        }
    }
}

void SizeReduceL2(const double eta, const int k, NTL::mat_RR &r, NTL::vec_RR &s)
{
    const double eta_bar = (eta + eta + 1) * 0.25;
    NTL::ZZ x;
    NTL::RR max;
    r[0][0] = Dot(lattice.basis[0], lattice.basis[0]);
    lattice.mu[0][0] = 1.0;

    for (long i, j, h;;)
    {
        for (j = 0; j <= k; ++j)
        {
            r[k][j] = Dot(lattice.basis[k], lattice.basis[j]);
            for (h = 0; h < j; ++h)
            {
                r[k][j] -= r[k][h] * lattice.mu[j][h];
            }
            lattice.mu[k][j] = r[k][j] / r[j][j];
        }

        s[0] = Dot(lattice.basis[k], lattice.basis[k]);
        for (j = 1; j <= k; ++j)
        {
            s[j] = s[j - 1] - lattice.mu[k][j - 1] * r[k][j - 1];
        }
        r[k][k] = s[k];

        max = -1;
        for (i = 0; i < k; ++i)
        {
            if ((lattice.mu[k][i] > max) || (lattice.mu[k][i] < -max))
            {
                max = NTL::abs(lattice.mu[k][i]);
            }
        }

        if (max > eta_bar)
        {
            for (i = k - 1; i >= 0; --i)
            {
                x = NTL::RoundToZZ(lattice.mu[k][i]);
                lattice.basis[k] -= x * lattice.basis[i];
                for (j = 0; j <= i; ++j)
                {
                    lattice.mu[k][j] -= NTL::to_RR(x) * lattice.mu[i][j];
                }
            }
        }
        else
        {
            break;
        }
    }
}

void L2Reduce(HWND hWnd, UINT Msg, const double delta)
{
    const double delta_bar = (delta + 1) * 0.5;
    double prog_ratio = 0.0;
    NTL::mat_RR r;
    NTL::vec_RR s;

    // Reduces basis lightly
    NTL::LLL_XD(lattice.basis);

    NTL::clear(lattice.mu);
    NTL::clear(lattice.B);
    r.SetDims(lattice.rank, lattice.rank);
    s.SetLength(lattice.rank + 1);
    r[0][0] = Dot(lattice.basis[0], lattice.basis[0]);

    for (long k = 1, k_, j; k < lattice.rank;)
    {
        if (k * 100.0 / (lattice.rank - 1.0) > prog_ratio)
        {
            prog_ratio = k * 100.0 / (lattice.rank - 1.0);
        }
        PostMessageA(hWnd, Msg, (int)std::round(prog_ratio), 0);

        SizeReduceL2(0.51, k, r, s);

        k_ = k;
        while ((k >= 1) && (delta_bar * r[k - 1][k - 1] >= s[k - 1]))
        {
            NTL::swap(lattice.basis[k], lattice.basis[k - 1]);
            --k;
        }

        for (j = 0; j < k; ++j)
        {
            lattice.mu[k][j] = lattice.mu[k_][j];
            r[k][j] = r[k_][j];
        }
        r[k][k] = s[k];

        ++k;
    }
}

void LLLReduce(const double delta, const int end, const int h)
{
    NTL::RR nu, B, t;
    NTL::vec_ZZ tmp;

    ComputeGSO();

    for (int k = h, i, j; k < end;)
    {
        for (j = k - 1; j > -1; --j)
        {
            SizeReduce(k, j);
        }

        if ((k > 0) && (lattice.B[k] < (delta - lattice.mu[k][k - 1] * lattice.mu[k][k - 1]) * lattice.B[k - 1]))
        {
            NTL::swap(lattice.basis[k], lattice.basis[k - 1]);

            nu = lattice.mu[k][k - 1];
            B = lattice.B[k] + nu * nu * lattice.B[k - 1];
            lattice.mu[k][k - 1] = nu * lattice.B[k - 1] / B;
            lattice.B[k] *= lattice.B[k - 1] / B;
            lattice.B[k - 1] = B;

            for (i = 0; i < k - 1; ++i)
            {
                t = lattice.mu[k - 1][i];
                lattice.mu[k - 1][i] = lattice.mu[k][i];
                lattice.mu[k][i] = t;
            }
            for (i = k + 1; i < end; ++i)
            {
                t = lattice.mu[i][k];
                lattice.mu[i][k] = lattice.mu[i][k - 1] - nu * t;
                lattice.mu[i][k - 1] = t + lattice.mu[k][k - 1] * lattice.mu[i][k];
            }

            --k;
        }
        else
        {
            ++k;
        }
    }
}

void DeepLLLReduce(HWND hWnd, UINT Msg, const double delta, const int end, const int h)
{
    bool flag;
    const int gamma = (lattice.rank >> 1);
    double prog_ratio = 0.0;
    NTL::RR C;

    NTL::LLL_XD(lattice.basis);
    ComputeGSO();

    for (int k = h, j, i; k < end;)
    {
        if (k * 100.0 / (end - 1.0) > prog_ratio)
        {
            prog_ratio = k * 100.0 / (end - 1.0);
        }
        PostMessageA(hWnd, Msg, (int)std::round(prog_ratio), 0);

        for (j = k - 1; j > -1; --j)
        {
            SizeReduce(k, j);
        }

        C = Dot(lattice.basis[k], lattice.basis[k]);
        for (i = 0; i < k;)
        {
            if ((i > -1) && (C < delta * lattice.B[i]))
            {
                flag = ((i - 1 <= gamma) || (k - i < gamma));
            }

            if (flag)
            {
                if (C >= delta * lattice.B[i])
                {
                    C -= lattice.mu[k][i] * lattice.mu[k][i] * lattice.B[i];
                    ++i;
                }
                else
                {
                    DeepInsertion(i, k);
                    UpdateDeepInsertionGSO(i, k, end);

                    k = std::max(i - 1, 0);
                }
            }
            else
            {
                C -= lattice.mu[k][i] * lattice.mu[k][i] * lattice.B[i];
                ++i;
            }
        }
        ++k;
    }
}