#include "reduction.h"

#include <NTL/ZZ.h>
#include <NTL/RR.h>
#include <NTL/vec_ZZ.h>
#include <NTL/mat_RR.h>
#include <NTL/vec_RR.h>
#include <NTL/LLL.h>

#include "core.h"
#include "lattice.h"

double delta = 0.99;
int gamma;
int beta;
int max_loop = 5;

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

void L2Reduce(HWND hWnd, UINT Msg)
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
        while ((k > 0) && (delta_bar * r[k - 1][k - 1] >= s[k - 1]))
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

void LLLReduce(const int end, const int h)
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

void DeepLLLReduce(HWND hWnd, UINT Msg, const int end, const int h)
{
    bool flag;
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

void PotLLLReduce(HWND hWnd, UINT Msg, const int end, const int h)
{
    NTL::RR P, P_min, S;
    double prog_ratio = 0.0;

    NTL::LLL_XD(lattice.basis);
    ComputeGSO();

    for (int l = 0, j, i, k; l < lattice.rank;)
    {
        if (l * 100.0 / (end - 1.0) > prog_ratio)
        {
            prog_ratio = l * 100.0 / (end - 1.0);
        }
        PostMessageA(hWnd, Msg, (int)std::round(prog_ratio), 0);

        for (j = l - 1; j > -1; --j)
        {
            SizeReduce(l, j);
        }

        P = P_min = 1.0;
        k = 0;
        for (j = l - 1; j >= 0; --j)
        {
            S = 0;
            for (i = j; i < l; ++i)
            {
                S += lattice.mu[l][i] * lattice.mu[l][i] * lattice.B[i];
            }
            P *= (lattice.B[l] + S) / lattice.B[j];

            if (P < P_min)
            {
                k = j;
                P_min = P;
            }
        }

        if (delta > P_min)
        {
            DeepInsertion(k, l);
            UpdateDeepInsertionGSO(k, l, lattice.rank);
            l = k;
        }
        else
        {
            ++l;
        }
    }
}

void BKZ(HWND hWnd, UINT Msg)
{
    double prog_ratio_1 = 0.0;
    double prog_ratio_2 = 0.0;
    NTL::vec_ZZ v;
    NTL::vec_ZZ w;
    NTL::mat_ZZ tmp_mat;

    NTL::LLL_XD(lattice.basis);
    for (int i = 10; i <= beta; i += 2)
    {
        NTL::BKZ_FP(lattice.basis, 0.99, i);
    }
    ComputeGSO();

    for (int z = 0, k = 0, loops = 0, l, h, i, j; z < lattice.rank - 2;)
    {
        if (z * 100.0 / (lattice.rank - 2.0) > prog_ratio_1)
        {
            prog_ratio_1 = z * 100.0 / (lattice.rank - 2.0);
        }
        if (100.0 * loops / max_loop > prog_ratio_2)
        {
            prog_ratio_2 = 100.0 * loops / max_loop;
        }
        PostMessageA(hWnd, Msg, MAX(prog_ratio_1, prog_ratio_2), 0);

        if (loops >= max_loop)
        {
            break;
        }

        if (k == lattice.rank - 1)
        {
            k = 0;
            ++loops;
        }
        ++k;
        l = std::min(k + beta - 1, (int)lattice.rank);
        h = std::min(l + 1, (int)lattice.rank);
        if (ENUM(k - 1, l, w))
        {
            z = 0;

            v.SetLength(lattice.rank);
            for (i = 0; i < lattice.rank; ++i)
            {
                for (j = 0; j < w.length(); ++j)
                {
                    v[i] += w[j] * lattice.basis[j][i];
                }
            }

            tmp_mat.SetDims(h + 1, lattice.rank);
            for (j = 0; j < lattice.rank; ++j)
            {
                for (i = 0; i < k - 1; ++i)
                {
                    tmp_mat[i][j] = lattice.basis[i][j];
                }
                tmp_mat[k - 1][j] = v[j];
                for (i = k; i <= h; ++i)
                {
                    tmp_mat[i][j] = lattice.basis[i - 1][j];
                }
            }
            NTL::LLL_FP(tmp_mat);
            for (i = 1; i <= h; ++i)
            {
                for (j = 0; j < lattice.rank; ++j)
                {
                    lattice.basis[i - 1][j] = tmp_mat[i][j];
                }
            }
        }
        else
        {
            ++z;
            NTL::LLL_FP(lattice.basis);
            //            LLLReduce(k, 0);
        }

        ComputeGSO();
    }
}