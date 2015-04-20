#include <NTL/mat_ZZ_p.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <NTL/vec_ZZ_p.h>
#include <NTL/vec_ZZ.h>
#include <cmath>


using namespace std;
using namespace NTL;

const ZZ w(12345), q((1LL << 31LL) - 1LL);

vec_ZZ decrypt(mat_ZZ_p S, vec_ZZ_p c);

// returns c*
vec_ZZ_p getBitVector(vec_ZZ_p c);

// returns S*
mat_ZZ_p getBitMatrix(mat_ZZ_p S);

// returns S
mat_ZZ_p getSecretKey(mat_ZZ_p T);

// returns M
mat_ZZ_p keySwitchMatrix(mat_ZZ_p S, mat_ZZ_p T);

// finds c* then returns Mc*
vec_ZZ_p keySwitch(mat_ZZ_p M, vec_ZZ_p c);

// as described, treating I as the secret key and wx as ciphertext
vec_ZZ_p encrypt(mat_ZZ_p T, vec_ZZ_p x);


mat_ZZ_p getRandomMatrix(int m, int n);


vec_ZZ decrypt(mat_ZZ_p S, vec_ZZ_p c) {
	vec_ZZ_p Sc = S*c;
	vec_ZZ output;
	output.SetLength(Sc.length());
	for (unsigned int i=0; i<Sc.length(); i++) {
		output[i] = (rep(Sc[i])+w/2)/w;
	}
	return output;
}

int main()
{
	ZZ_p::init(q);
   return 0;
}

