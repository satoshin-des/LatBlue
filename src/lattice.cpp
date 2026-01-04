#include <lattice.h>

#include <vector>

#include <NTL/RR.h>
#include <NTL/mat_RR.h>
#include <NTL/LLL.h>

Lattice lattice;

void ComputeGSO()
{
    NTL::mat_RR gsoB;
    gsoB.SetDims(lattice.rank, lattice.rank);
    lattice.mu.SetDims(lattice.rank, lattice.rank);
    lattice.B.SetLength(lattice.rank);
    NTL::clear(gsoB);

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
    std::vector<NTL::RR> P(n), D(n), S(n);

    P[k] = D[k] = lattice.B[k];
    for (j = k - 1; j >= i; --j)
    {
        P[j] = lattice.mu[k][j] * lattice.B[j];
        D[j] = D[j + 1] + lattice.mu[k][j] * P[j];
    }

    for (j = k; j > i; --j)
    {
        t = lattice.mu[k][j - 1] / D[j];
        for (l = end - 1; l > k; --l)
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

    for (l = end - 1; l > k; --l)
    {
        lattice.mu[l][i] = t * (S[l] + lattice.mu[l][i] * P[i]);
    }
    for (l = k; l >= i + 2; --l)
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
