#include <iostream>
#include <sstream>
#include <cassert>
//#include <NTL/mat_ZZ_p.h>
#include <NTL/mat_ZZ.h>
//#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
//#include <NTL/vec_ZZ_p.h>
#include <NTL/vec_ZZ.h>
#include <cmath>
#include <stack>

using namespace std;
using namespace NTL;

const ZZ twoToTheTwenty(1 << 20);
const ZZ w(twoToTheTwenty*twoToTheTwenty);
ZZ aBound(10000), tBound(aBound), eBound(10000);
int l = 100;

mat_ZZ hCat(mat_ZZ A, mat_ZZ B);
mat_ZZ vCat(mat_ZZ A, mat_ZZ B);

vec_ZZ decrypt(mat_ZZ S, vec_ZZ c);

// returns c*
vec_ZZ getBitVector(vec_ZZ c);

// returns S*
mat_ZZ getBitMatrix(mat_ZZ S);

// returns S
mat_ZZ getSecretKey(mat_ZZ T);

// returns M
mat_ZZ keySwitchMatrix(mat_ZZ S, mat_ZZ T);

// finds c* then returns Mc*
vec_ZZ keySwitch(mat_ZZ M, vec_ZZ c);

// as described, treating I as the secret key and wx as ciphertext
vec_ZZ encrypt(mat_ZZ T, vec_ZZ x);

mat_ZZ getRandomMatrix(long row, long col, ZZ bound);

// server side addition with same secret key
vec_ZZ addn(vec_ZZ c1, vec_ZZ c2);

// server side linear transformation,
// returns S(Gx) given c=Sx and M (key switch matrix from GS to S)
vec_ZZ linearTransform(mat_ZZ M, vec_ZZ c);

// returns M, the key switch matrix from GS to S,
// to be sent to server
mat_ZZ linearTransformClient(mat_ZZ T, mat_ZZ G);

// computes an inner product, given two ciphertexts and the keyswitch matrix
vec_ZZ innerProd(vec_ZZ c1, vec_ZZ c2, mat_ZZ M);

// returns M, the key switch matrix from vec(S^t S) to S,
// to be sent to the server
mat_ZZ innerProdClient(mat_ZZ T);

// returns a column vector
mat_ZZ vectorize(mat_ZZ M);

mat_ZZ copyRows(mat_ZZ row, long numrows);




// finds c* then returns Mc*
vec_ZZ keySwitch(mat_ZZ M, vec_ZZ c){
	vec_ZZ cstar = getBitVector(c);
	return M * cstar;
}


mat_ZZ getRandomMatrix(long row, long col, ZZ bound){
	mat_ZZ A;
	A.SetDims(row, col);
	for (int i=0; i<row; ++i){
		for (int j=0; j<col; ++j){
			A[i][j] = RandomBnd(bound);
		}
	}
	return A;
}




// returns S*
mat_ZZ getBitMatrix(mat_ZZ S) {
	mat_ZZ result;
	int rows = S.NumRows(), cols = S.NumCols();
	result.SetDims(rows, l * cols);

	vec_ZZ powers;
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
vec_ZZ getBitVector(vec_ZZ c) {
	vec_ZZ result;
	int length = c.length();
	result.SetLength(length * l);
	for(int i = 0; i < length; ++i) {
		ZZ sign = (c[i] < ZZ(0)) ? ZZ(-1) : ZZ(1);
		ZZ value = c[i] * sign;
		for(int j = 0; j < l; ++j) {
			result[i * l + j] = sign*bit(value, j);
		}
	}
	return result;
}



// returns S
mat_ZZ getSecretKey(mat_ZZ T) {
	mat_ZZ I;
	ident(I, T.NumRows());
	return hCat(I, T);
}


mat_ZZ hCat(mat_ZZ A, mat_ZZ B) {
	assert(A.NumRows() == B.NumRows());

	int rows = A.NumRows(), colsA = A.NumCols(), colsB = B.NumCols();
	mat_ZZ result;
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

mat_ZZ vCat(mat_ZZ A, mat_ZZ B) {
	assert(A.NumCols() == B.NumCols());

	int cols = A.NumCols(), rowsA = A.NumRows(), rowsB = B.NumRows();
	mat_ZZ result;
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

ZZ nearestInteger(ZZ x, ZZ w) {
	return (x + (w+1)/2) / w;
}

vec_ZZ decrypt(mat_ZZ S, vec_ZZ c) {
	vec_ZZ Sc = S*c;
	vec_ZZ output;
	output.SetLength(Sc.length());
	for (int i=0; i<Sc.length(); i++) {
		output[i] = nearestInteger(Sc[i], w);
	}
	return output;
}

mat_ZZ keySwitchMatrix(mat_ZZ S, mat_ZZ T) {
	mat_ZZ Sstar = getBitMatrix(S);
	mat_ZZ A = getRandomMatrix(T.NumCols(),Sstar.NumCols(),aBound);
	mat_ZZ E = getRandomMatrix(Sstar.NumRows(),Sstar.NumCols(),eBound);
	mat_ZZ M = vCat(Sstar + E - T*A, A);
	return M;
}

vec_ZZ encrypt(mat_ZZ T, vec_ZZ x) {
	mat_ZZ I;
	ident(I, x.length());
	return keySwitch(keySwitchMatrix(I, T), w * x);
}




vec_ZZ addVectors(vec_ZZ c1, vec_ZZ c2){
	return c1 + c2;
}

vec_ZZ linearTransform(mat_ZZ M, vec_ZZ c){
	return M * getBitVector(c);
}

mat_ZZ linearTransformClient(mat_ZZ T, mat_ZZ G){
	mat_ZZ S = getSecretKey(T);
	return keySwitchMatrix(G * S, T);
}


vec_ZZ innerProd(vec_ZZ c1, vec_ZZ c2, mat_ZZ M){
	mat_ZZ cc1;
	mat_ZZ cc2;
	mat_ZZ cc;

	cc1.SetDims(c1.length(), 1);
	for (int i=0; i<c1.length(); ++i){
		cc1[i][0] = c1[i];
	}
	cc2.SetDims(1, c2.length());
	for (int i=0; i<c2.length(); ++i){
		cc2[0][i] = c2[i];
	}
	cc = vectorize(cc1 * cc2);

	vec_ZZ output;
	output.SetLength(cc.NumRows());
	for (int i=0; i<cc.NumRows(); i++) {
		output[i] = nearestInteger(cc[i][0], w);
	}
	return M * getBitVector(output);
}

mat_ZZ innerProdClient(mat_ZZ T){
	mat_ZZ S = getSecretKey(T);
	mat_ZZ tvsts = transpose(vectorize(transpose(S) * S));
	mat_ZZ mvsts = copyRows(tvsts, T.NumRows());
	return keySwitchMatrix(mvsts, T);
}




mat_ZZ copyRows(mat_ZZ row, long numrows){
	mat_ZZ ans;
	ans.SetDims(numrows, row.NumCols());
	for (int i=0; i<ans.NumRows(); ++i){
		for (int j=0; j<ans.NumCols(); ++j){
			ans[i][j] = row[0][j];
		}
	}
	return ans;
}

mat_ZZ vectorize(mat_ZZ M){
	mat_ZZ ans;
	ans.SetDims(M.NumRows() * M.NumCols(), 1);
	for (int i=0; i<M.NumRows(); ++i){
		for (int j=0; j<M.NumCols(); ++j){
			ans[i*M.NumCols() + j][0] = M[i][j];
		}
	}
	return ans;
}



int main() {
	stack<vec_ZZ> vectors;
	stack<mat_ZZ> matrices;

	string operation;
	while (cin >> operation) {

		if (operation == "vector") {
			vec_ZZ v;
			cin >> v;
			vectors.push(v);

		} else if (operation == "matrix") {
			mat_ZZ m;
			cin >> m;
			matrices.push(m);

		} else if (operation == "duplicate-vector") {
			vectors.push(vectors.top());

		} else if (operation == "duplicate-matrix") {
			matrices.push(matrices.top());

		} else if (operation == "add") {
			vec_ZZ v1 = vectors.top(); vectors.pop();
			vec_ZZ v2 = vectors.top(); vectors.pop();
			vectors.push(addVectors(v1, v2));

		} else if (operation == "scalar-multiply") {
			ZZ x;
			cin >> x;
			vec_ZZ v = vectors.top(); vectors.pop();
			vectors.push(v * x);

		} else if (operation == "linear-transform") {
			vec_ZZ v = vectors.top(); vectors.pop();
			mat_ZZ m = matrices.top(); matrices.pop();
			vectors.push(linearTransform(m, v));

		} else if (operation == "linear-transform-key-switch") {
			mat_ZZ T = matrices.top(); matrices.pop();
			mat_ZZ G = matrices.top(); matrices.pop();
			matrices.push(linearTransformClient(T, G));

		} else if (operation == "inner-product") {
			vec_ZZ v1 = vectors.top(); vectors.pop();
			vec_ZZ v2 = vectors.top(); vectors.pop();
			mat_ZZ m = matrices.top(); matrices.pop();
			vectors.push(innerProd(v1, v2, m));

		} else if (operation == "inner-product-key-switch") {
			mat_ZZ T = matrices.top(); matrices.pop();
			matrices.push(innerProdClient(T));

		} else if (operation == "key-switch") {
			vec_ZZ v = vectors.top(); vectors.pop();
			mat_ZZ m = matrices.top(); matrices.pop();
			vectors.push(keySwitch(m, v));

		} else if (operation == "random-matrix") {
			int dimension;
			cin >> dimension;
			matrices.push(getRandomMatrix(dimension, dimension, tBound));

		} else if (operation == "identity") {
			int rows;
			cin >> rows;
			mat_ZZ I;
			ident(I, rows);
			matrices.push(I);

		} else if (operation == "key-switch-matrix") {
			mat_ZZ T = matrices.top(); matrices.pop();
			mat_ZZ S = matrices.top(); matrices.pop();
			matrices.push(keySwitchMatrix(S, T));

		} else if (operation == "get-secret-key") {
			mat_ZZ T = matrices.top(); matrices.pop();
			matrices.push(getSecretKey(T));

		} else if (operation == "encrypt") {
			mat_ZZ T = matrices.top(); matrices.pop();
			vec_ZZ x = vectors.top(); vectors.pop();
			vectors.push(encrypt(T, x));

		} else if (operation == "decrypt") {
			mat_ZZ S = matrices.top(); matrices.pop();
			vec_ZZ c = vectors.top(); vectors.pop();
			vectors.push(decrypt(S, c));

		} else {
			cerr << "Unknown command: " << operation << endl;
		}

	}

	stack<vec_ZZ> vectors2;
	while (vectors.size()) {
		vectors2.push(vectors.top()); vectors.pop();
	}
	while (vectors2.size()) {
		cout << vectors2.top() << endl; vectors2.pop();
	}

	stack<mat_ZZ> matrices2;
	while (matrices.size()) {
		matrices2.push(matrices.top()); matrices.pop();
	}
	while (matrices2.size()) {
		cout << matrices2.top() << endl; matrices2.pop();
	}


	// // Testing for the 3 fundamental operations:
	// const int N = 10;
	// vec_ZZ x1;
	// vec_ZZ x2;
	// x1.SetLength(N);
	// x2.SetLength(N);
	// for(int i = 0; i < N; ++i) {
	// 	x1[i] = RandomBnd(10000);
	// 	x2[i] = RandomBnd(10000);
	// }
	// cout << x1 << endl;
	// cout << x2 << endl;
	// mat_ZZ T = getRandomMatrix(N, N, tBound);
	// mat_ZZ S = getSecretKey(T);
	// vec_ZZ c1 = encrypt(T, x1);
	// vec_ZZ c2 = encrypt(T, x2);



	// // Testing for inner product no switch
	// vec_ZZ cc = innerProdNoSwitch(x1, c2);
	// vec_ZZ dxx = innerProdNoSwitchDecrypt(cc, S);
	// ZZ xx;
	// InnerProduct(xx, x1, x2);

	// cout << xx << endl;
	// cout << dxx[0] << endl;
	// cout << xx - dxx[0] << endl;


	// // Testing for inner product
	// mat_ZZ M;
	// vec_ZZ cc;
	// vec_ZZ dxx;
	// ZZ xx;

	// M = innerProdClient(T);
	// cc = innerProd(c1, c2, M);
	// dxx = decrypt(getSecretKey(T), cc);
	// InnerProduct(xx, x1, x2);


	// cout << xx << endl;
	// cout << dxx[0] << endl;
	// cout << xx - dxx[0] << endl;


	// // Testing for linear transform
	// mat_ZZ G;
	// mat_ZZ M;
	// vec_ZZ cc;
	// vec_ZZ dxx;
	// vec_ZZ xx;

	// G = getRandomMatrix(N, N, aBound);
	// M = linearTransformClient(T, G);
	// cc = linearTransform(M, c1);
	// dxx = decrypt(getSecretKey(T), cc);
	// xx = G * x1;

	// cout << G << endl;
	// cout << xx << endl;
	// cout << dxx << endl;
	// cout << xx - dxx << endl;



	// // Testing for addition:
	// vec_ZZ cplus;
	// vec_ZZ dxplus;
	// vec_ZZ xplus;

	// cplus = addn(c1, c2);
	// dxplus = decrypt(getSecretKey(T), cplus);
	// xplus = x1 + x2;

	// cout << xplus << endl;
	// cout << dxplus << endl;
	// cout << xplus - dxplus << endl;


}

