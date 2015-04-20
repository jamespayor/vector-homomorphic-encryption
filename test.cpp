#include <cassert>
#include <NTL/mat_ZZ_p.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <NTL/vec_ZZ_p.h>
#include <NTL/vec_ZZ.h>
#include <cmath>


using namespace std;
using namespace NTL;

const ZZ w(12345), q((1LL << 31LL) - 1LL);
const ZZ aBound(12345), eBound(100);
const int l = 34;

vec_ZZ decrypt(mat_ZZ_p S, vec_ZZ_p c);

mat_ZZ_p hCat(mat_ZZ_p A, mat_ZZ_p B);
mat_ZZ_p vCat(mat_ZZ_p A, mat_ZZ_p B);

// returns c*
vec_ZZ_p getBitVector(vec_ZZ_p c);

// returns S*
mat_ZZ_p getBitMatrix(mat_ZZ_p S);

// returns S
mat_ZZ_p getSecretKey(mat_ZZ_p T);

// returns M
mat_ZZ_p keySwitchMatrix(mat_ZZ_p S, mat_ZZ_p T, ZZ Abound, ZZ Ebound);

// finds c* then returns Mc*
vec_ZZ_p keySwitch(mat_ZZ_p M, vec_ZZ_p c);

// as described, treating I as the secret key and wx as ciphertext
vec_ZZ_p encrypt(mat_ZZ_p T, vec_ZZ x);


mat_ZZ_p getRandomMatrix(long row, long col, long bound);






// finds c* then returns Mc*
vec_ZZ_p keySwitch(mat_ZZ_p M, vec_ZZ_p c){
    vec_ZZ_p cstar = getBitVector(c);
    return M * cstar;
}


mat_ZZ_p getRandomMatrix(long row, long col, long bound){
    mat_ZZ_p A;
    A.SetDims(row, col);
    for (int i=0; i<row; ++i){
        for (int j=0; j<col; ++j){
            A[i][j] = (ZZ_p)RandomBnd(bound);
        }
    }
    return A;
}




// returns S*
mat_ZZ_p getBitMatrix(mat_ZZ_p S) {
	mat_ZZ_p result;
	int rows = S.NumRows(), cols = S.NumCols();
	result.SetDims(rows, l * cols);
	
	vec_ZZ_p powers;
	powers.SetLength(l);
	powers[0] = 1;
	for(int i = 0; i < l - 1; ++i) {
		powers[i+1] = powers[i]*2;
	}

	for(int i = 0; i < rows; ++i) {
		for(int j = 0; j < cols; ++j) {
			for(int k = 0; k < l; ++k) {
				result[i][j*l + k] = S[i][j] * powers[k];
			}
		}
	}

	return result;
}


// returns c*
vec_ZZ_p getBitVector(vec_ZZ_p c) {
	vec_ZZ_p result;
	int length = c.length();
	result.SetLength(length * l);
	for(int i = 0; i < length; ++i) {
		ZZ value = rep(c[i]);
		for(int j = 0; j < l; ++j) {
			result[i * l + j] = bit(value, j);
		}
	}
	return result;
}



// returns S
mat_ZZ_p getSecretKey(mat_ZZ_p T) {
	mat_ZZ_p I;
	ident(I, T.NumRows());
	return hCat(I, T);
}


mat_ZZ_p hCat(mat_ZZ_p A, mat_ZZ_p B) {
	assert(A.NumRows() == B.NumRows());

	int rows = A.NumRows(), colsA = A.NumCols(), colsB = B.NumCols();
	mat_ZZ_p result;
	result.SetDims(rows, colsA + colsB);

	// Copy A
	for(int i = 0; i < rows; ++i) {
		for(int j = 0; j < colsA; ++j) {
			result[i][j] = A[i][j];
		}
	}

	// Copy B
	for(int i = 0; i < rows; ++i) {
		for(int j = 0; j < colsB; ++j) {
			result[i][colsA + j] = B[i][j];
		}
	}

	return result;
}

mat_ZZ_p vCat(mat_ZZ_p A, mat_ZZ_p B) {
	assert(A.NumCols() == B.NumCols());

	int cols = A.NumCols(), rowsA = A.NumRows(), rowsB = B.NumRows();
	mat_ZZ_p result;
	result.SetDims(rowsA + rowsB, cols);

	// Copy A
	for(int i = 0; i < rowsA; ++i) {
		for(int j = 0; j < cols; ++j) {
			result[i][j] = A[i][j];
		}
	}

	// Copy B
	for(int i = 0; i < rowsB; ++i) {
		for(int j = 0; j < cols; ++j) {
			result[i + rowsA][j] = B[i][j];
		}
	}

	return result;
}


vec_ZZ decrypt(mat_ZZ_p S, vec_ZZ_p c) {
	vec_ZZ_p Sc = S*c;
	vec_ZZ output;
	output.SetLength(Sc.length());
	for (int i=0; i<Sc.length(); i++) {
		output[i] = (rep(Sc[i])+w/2)/w;
	}
	return output;
}

mat_ZZ_p keySwitchMatrix(mat_ZZ_p S, mat_ZZ_p T, ZZ Abound, ZZ Ebound) {
	mat_ZZ_p Sstar = getBitMatrix(S);
	mat_ZZ_p A = getRandomMatrix(T.NumCols(),Sstar.NumCols(),Abound);
	mat_ZZ_p E = getRandomMatrix(Sstar.NumRows(),Sstar.NumCols(),Ebound);
	mat_ZZ_p M = vCat(Sstar + E - T*A, A);
	return Sstar;
}

vec_ZZ_p encrypt(mat_ZZ_p T, vec_ZZ x) {
	mat_ZZ_p I;
	ident(I, x.length());
	return keySwitch(keySwitchMatrix(I, T, aBound, eBound), conv<vec_ZZ_p>(w * x));
}


int main()
{
	ZZ_p::init(q);
   return 0;
}

