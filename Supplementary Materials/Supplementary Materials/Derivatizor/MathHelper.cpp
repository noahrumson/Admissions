// Noah Rubin

#include <cmath>

#include "MathHelper.h"

int BinomialCoefficient(int n, int k)
{
	double res = 1;	// we can use integer arithmetic if we recognize the symmetry of Pascal's triangle
	for (int i = 1; i <= k; ++i) {
		res *= (n + 1 - i) / i;
	}
	return (int) res;
}

std::vector<int> BinomialCoefficients(int n)
{
	std::vector<int> res(n + 1);
	res[0] = 1;	// we can use integer arithmetic if we recognize the symmetry of Pascal's triangle
	for (int i = 1; i <= n; ++i) {
		res[i] = res[i - 1] * (n + 1 - i) / i;
	}
	return res;
}
