#include "core.h"

#include <iostream>
#include <sstream>

#include <NTL/ZZ.h>

#include "tools.h"

NTL::mat_ZZ B;
int n;

void generator(const int seed)
{
    const long bit = 10L;

    NTL::vec_ZZ v;
    generate_random_HNF(v, n, bit, NTL::to_ZZ(seed));
    B.SetDims(n, n);
    NTL::clear(B);
    B(1, 1) = v(1);
    for (int i = 2; i <= n; i++)
    {
        B(i, 1) = v(i);
        B(i, i) = 1;
    }
}
