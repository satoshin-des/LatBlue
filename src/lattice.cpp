#include <lattice.h>

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
