#include <lattice.h>

#include <vector>

#include <NTL/RR.h>
#include <NTL/mat_RR.h>
#include <NTL/LLL.h>

#include "core.h"
#include "reduction.h"

Lattice lattice;

void ComputeGSO()
{
    NTL::mat_RR gsoB;
    gsoB.SetDims(lattice.rank, lattice.rank);
    lattice.mu.SetDims(lattice.rank, lattice.rank);
    lattice.B.SetLength(lattice.rank);
    NTL::clear(gsoB);
    NTL::clear(lattice.mu);
    NTL::clear(lattice.B);

    for (int i = 1, j; i <= lattice.rank; ++i)
    {
        lattice.mu(i, i) = 1.0;
        gsoB(i) = NTL::conv<NTL::vec_RR>(lattice.basis(i));
        for (j = 1; j < i; ++j)
        {
            NTL::InnerProduct(lattice.mu(i, j), NTL::conv<NTL::vec_RR>(lattice.basis(i)), gsoB(j));
            lattice.mu(i, j) /= lattice.B(j);
            gsoB(i) -= lattice.mu(i, j) * gsoB(j);
        }
        NTL::InnerProduct(lattice.B(i), gsoB(i), gsoB(i));
    }
}

void DeepInsertion(const int k, const int l)
{
    NTL::vec_ZZ t;
    t = lattice.basis[l];
    for (int j = l; j > k; --j)
    {
        lattice.basis[j] = lattice.basis[j - 1];
    }
    lattice.basis[k] = t;
}

void UpdateDeepInsertionGSO(const long i, const long k, const long end)
{
    int j, l;
    const int n = end;
    NTL::RR t, eps;
    NTL::vec_RR P, D, S;
    P.SetLength(n);
    D.SetLength(n);
    S.SetLength(n);

    P[k] = D[k] = lattice.B[k];
    for (j = k - 1; j >= i; --j)
    {
        P[j] = lattice.mu[k][j] * lattice.B[j];
        D[j] = D[j + 1] + lattice.mu[k][j] * P[j];
    }

    for (j = k; j > i; --j)
    {
        t = lattice.mu[k][j - 1] / D[j];
        for (l = n - 1; l > k; --l)
        {
            S[l] += lattice.mu[l][j] * P[j];
            lattice.mu[l][j] = lattice.mu[l][j - 1] - t * S[l];
        }
        for (l = k; l > j; --l)
        {
            S[l] += lattice.mu[l - 1][j] * P[j];
            lattice.mu[l][j] = lattice.mu[l - 1][j - 1] - t * S[l];
        }
    }

    t = 1.0 / D[i];

    for (l = n - 1; l > k; --l)
    {
        lattice.mu[l][i] = t * (S[l] + lattice.mu[l][i] * P[i]);
    }
    for (l = k; l > i + 1; --l)
    {
        lattice.mu[l][i] = t * (S[l] + lattice.mu[l - 1][i] * P[i]);
    }

    lattice.mu[i + 1][i] = t * P[i];
    for (j = 0; j < i; ++j)
    {
        eps = lattice.mu[k][j];
        for (l = k; l > i; --l)
        {
            lattice.mu[l][j] = lattice.mu[l - 1][j];
        }
        lattice.mu[i][j] = eps;
    }

    for (j = k; j > i; --j)
    {
        lattice.B[j] = D[j] * lattice.B[j - 1] / D[j - 1];
    }
    lattice.B[i] = D[i];
}

NTL::ZZ Volume()
{
    return NTL::abs(NTL::determinant(lattice.basis));
}

NTL::RR ComputeSlope()
{
    NTL::RR S = NTL::to_RR(0.0), T = NTL::to_RR(0.0);
    for (int i = 1; i <= lattice.rank; ++i)
    {
        S += i * NTL::log(lattice.B(i));
        T += NTL::log(lattice.B(i));
    }
    return 12.0 * (S - 0.5 * (lattice.rank + 1) * T) / (lattice.rank * (lattice.rank * lattice.rank - 1));
}

NTL::RR ComputeRHF()
{
    const NTL::RR hf = NTL::sqrt(Dot(lattice.basis[0], lattice.basis[0])) / NTL::exp(NTL::log(NTL::to_RR(Volume())) / lattice.rank);
    return NTL::exp(NTL::log(hf) / lattice.rank);
}

NTL::RR LogPot()
{
    NTL::RR logp;
    logp = 0;
    for (int i = 0; i < lattice.rank; ++i)
    {
        logp += (lattice.rank - i) * NTL::log(lattice.B[i]);
    }
    return logp;
}

bool ENUM(const int start, const int end, NTL::vec_ZZ &v)
{
    bool has_solution = false; // if found a shortest vector or not
    const int d = end - start; // dimension of local projected block lattice

    int i, r[d + 1];
    NTL::RR tmp_val;     // temporary of NTL::RR
    NTL::vec_RR radius;  // radius for searching
    NTL::vec_ZZ tmp_vec; // temporary of NTL::vec_ZZ
    NTL::vec_ZZ width;   // width for zigzag-searching
    NTL::vec_RR eps;     // width for radius for searching
    NTL::vec_RR center;  // center of zigzag-searching
    NTL::vec_RR rho;     // squared norm of projected lattice vector
    NTL::mat_RR sigma;

    radius.SetLength(d);
    width.SetLength(d);
    center.SetLength(d);
    rho.SetLength(d + 1);
    sigma.SetDims(d + 1, d);
    v.SetLength(d);
    eps.SetLength(d);
    tmp_vec.SetLength(d);
    tmp_vec[0] = 1;
    for (i = 0; i < d; ++i)
    {
        r[i] = i;
        eps[i] = 0.99;
    }

    for (i = 0; i < d; ++i)
    {
        radius[i] = eps[d - i - 1] * lattice.B[start];
    }

    for (int k = 0, last_nonzero = 0;;)
    {
        tmp_val = NTL::to_RR(tmp_vec[k]) - center[k];
        tmp_val *= tmp_val;
        rho[k] = rho[k + 1] + tmp_val * lattice.B[k + start];
        if (rho[k] <= radius[d - k - 1])
        {
            if (k == 0)
            {
                has_solution = true;
                for (i = 0; i < d; ++i)
                {
                    v[i] = tmp_vec[i];
                }
                for (i = 0; i < d; ++i)
                {
                    radius[i] = 0.99 * MIN(0.99 * rho[0], radius[i]);
                }
            }
            else
            {
                --k;
                if (r[k + 1] > r[k])
                {
                    r[k] = r[k + 1];
                }
                for (i = r[k]; i > k; --i)
                {
                    sigma[i][k] = sigma[i + 1][k] + lattice.mu[i + start][k + start] * NTL::to_RR(tmp_vec[i]);
                }
                center[k] = -sigma[k + 1][k];
                tmp_vec[k] = NTL::RoundToZZ(center[k]);
                width[k] = 1;
            }
        }
        else
        {
            ++k;
            if (k == d)
            {
                return has_solution;
            }
            else
            {
                r[k] = k;
                if (k >= last_nonzero)
                {
                    last_nonzero = k;
                    ++tmp_vec[k];
                }
                else
                {
                    NTL::to_RR(tmp_vec[k]) > center[k] ? tmp_vec[k] -= width[k] : tmp_vec[k] += width[k];
                    ++width[k];
                }
            }
        }
    }
}

bool PotENUM(const int start, const int d, NTL::vec_ZZ &v)
{
    long i, r[d + 1];
    NTL::RR R = NTL::log(lattice.B[start]), P = NTL::to_RR(0), temp;
    NTL::vec_ZZ w;
    NTL::vec_RR c, D;
    NTL::mat_RR sigma;

    w.SetLength(d);
    c.SetLength(d);
    D.SetLength(d + 1);
    sigma.SetDims(d + 1, d);
    v.SetLength(d);
    NTL::clear(v);
    v[0] = 1;

    for (i = 0; i <= d; ++i)
    {
        r[i] = i;
    }
    for (int k = 0, last_nonzero = 0;;)
    {
        temp = NTL::to_RR(v[k]) - c[k];
        temp *= temp;
        D[k] = D[k + 1] + temp * lattice.B[k + start];
        if ((k + 1) * NTL::log(D[k]) + P < (k + 1) * std::log(delta) + R)
        {
            if (k == 0)
            {
                return true;
            }
            else
            {
                P += NTL::log(D[k]);
                --k;
                if (r[k] < r[k + 1])
                {
                    r[k] = r[k + 1];
                }
                for (i = r[k]; i > k; --i)
                {
                    sigma[i][k] = sigma[i + 1][k] + lattice.mu[i + start][k + start] * NTL::to_RR(v[i]);
                }
                c[k] = -sigma[k + 1][k];
                v[k] = NTL::RoundToZZ(c[k]);
                w[k] = 1;
            }
        }
        else
        {
            ++k;
            if (k == d)
            {
                return false;
            }
            else
            {
                r[k - 1] = k;
                if (k >= last_nonzero)
                {
                    last_nonzero = k;
                    ++v[k];
                    if (v[last_nonzero] >= 2)
                    {
                        ++k;
                        if (k == d)
                        {
                            return false;
                        }
                        else
                        {
                            r[k - 1] = k;
                            last_nonzero = k;
                            v[last_nonzero] = 1;
                        }
                    }
                    P = 0;
                    R = 0;
                    for (i = 0; i <= last_nonzero; ++i)
                    {
                        R += NTL::log(lattice.B[i + start]);
                    }
                }
                else
                {
                    if (NTL::to_RR(v[k]) > c[k])
                    {
                        v[k] -= w[k];
                    }
                    else
                    {
                        v[k] += w[k];
                    }
                    ++w[k];
                    P -= NTL::log(D[k]);
                }
            }
        }
    }
}