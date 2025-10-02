#ifndef __PRIMES_H__
#define __PRIMES_H__

namespace Prime
{

	void FindDivisors(int, std::vector<int>&);
	int FindLCM(int, int);
	int FindGCD(int, int);

}


#else
#error File already included
#endif
