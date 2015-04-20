#include <cassert>
#include <NTL/mat_ZZ_p.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <NTL/vec_ZZ_p.h>
#include <NTL/vec_ZZ.h>


using namespace std;
using namespace NTL;

const ZZ w(123456789), l(34);

vec_ZZ_p decrypt(mat_ZZ_p S, vec_ZZ_p c);

mat_ZZ_p hCat(mat_ZZ_p A, mat_ZZ_p B);
mat_ZZ_p vCat(mat_ZZ_p A, mat_ZZ_p B);

// returns c*
vec_ZZ_p getBitVector(vec_ZZ_p c);
//
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


mat_ZZ_p getRandomMatrix(int row, int col, int bound);






// finds c* then returns Mc*
vec_ZZ_p keySwitch(mat_ZZ_p M, vec_ZZ_p c){
    vec_ZZ_p cstar = getBitVector(c);
    return M * cstar;
}


mat_ZZ_p getRandomMatrix(int row, int col, int bound){
    mat_ZZ_p A;
    A.SetDims(row, col);
    for (int i=0; i<row; ++i){
        for (int j=0; j<col; ++j){
            A[i][j] = (ZZ_p)RandomBnd((long)bound);
        }
    }
    return A;
}




// returns c*
vec_ZZ_p getBitVector(vec_ZZ_p c) {
//	vec_ZZ_p result;
//	result.SetLength(c.length() * l);
//	for(int i = 0; i < c.length())
}



// returns S
mat_ZZ_p getSecretKey(mat_ZZ_p T) {

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

int main()
{

   return 0;
}

