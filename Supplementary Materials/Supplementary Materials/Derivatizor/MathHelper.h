// Noah Rubin

#ifndef MATH_HELPER_H_INCLUDED
#define MATH_HELPER_H_INCLUDED

#include <vector>

template<class T>
inline bool AlmostEqual(T x, T y)
{
	return std::abs(x - y) < std::numeric_limits<T>::epsilon();
}

int BinomialCoefficient(int n, int k);

std::vector<int> BinomialCoefficients(int n);

#endif // !MATH_HELPER_H_INCLUDED
