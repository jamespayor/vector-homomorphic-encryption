#include <iostream>
#include <fstream>
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

const ZZ w(1ll << 40ll);
ZZ aBound(1000), tBound(aBound), eBound(1000);
int l = 100;

const mat_ZZ hCat(const mat_ZZ& A, const mat_ZZ& B);
const mat_ZZ vCat(const mat_ZZ& A, const mat_ZZ& B);

const vec_ZZ decrypt(const mat_ZZ& S, const vec_ZZ& c);

// returns c*
const vec_ZZ getBitVector(const vec_ZZ& c);

// returns S*
const mat_ZZ getBitMatrix(const mat_ZZ& S);

// returns S
const mat_ZZ getSecretKey(const mat_ZZ& T);

// returns M
const mat_ZZ keySwitchMatrix(const mat_ZZ& S, const mat_ZZ& T);

// finds c* then returns Mc*
const vec_ZZ keySwitch(const mat_ZZ& M, const vec_ZZ& c);

// as described, treating I as the secret key and wx as ciphertext
const vec_ZZ encrypt(const mat_ZZ& T, const vec_ZZ& x);

const mat_ZZ getRandomMatrix(long row, long col, const ZZ& bound);

// server side addition with same secret key
const vec_ZZ addn(const vec_ZZ& c1, const vec_ZZ& c2);

// server side linear transformation,
// returns S(Gx) given c=Sx and M (key switch matrix from GS to S)
const vec_ZZ linearTransform(const mat_ZZ& M, const vec_ZZ& c);

// returns M, the key switch matrix from GS to S,
// to be sent to server
const mat_ZZ linearTransformClient(const mat_ZZ& T, const mat_ZZ& G);

// computes an inner product, given two ciphertexts and the keyswitch matrix
const vec_ZZ innerProd(const vec_ZZ& c1, const vec_ZZ& c2, const mat_ZZ& M);

// returns M, the key switch matrix from vec(S^t S) to S,
// to be sent to the server
const mat_ZZ innerProdClient(const mat_ZZ& T);

// returns a column vector
const mat_ZZ vectorize(const mat_ZZ& M);

const mat_ZZ copyRows(const mat_ZZ& row, long numrows);




// finds c* then returns Mc*
const vec_ZZ keySwitch(const mat_ZZ& M, const vec_ZZ& c) {
	vec_ZZ cstar = getBitVector(c);
	return M * cstar;
}


const mat_ZZ getRandomMatrix(long row, long col, const ZZ& bound){
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
const mat_ZZ getBitMatrix(const mat_ZZ& S) {
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
const vec_ZZ getBitVector(const vec_ZZ& c) {
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
const mat_ZZ getSecretKey(const mat_ZZ& T) {
	mat_ZZ I;
	ident(I, T.NumRows());
	return hCat(I, T);
}


const mat_ZZ hCat(const mat_ZZ& A, const mat_ZZ& B) {
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

const mat_ZZ vCat(const mat_ZZ& A, const mat_ZZ& B) {
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

inline const ZZ nearestInteger(const ZZ& x, const ZZ& w) {
	return (x + (w+1)/2) / w;
}

const vec_ZZ decrypt(const mat_ZZ& S, const vec_ZZ& c) {
	vec_ZZ Sc = S*c;
	vec_ZZ output;
	output.SetLength(Sc.length());
	for (int i=0; i<Sc.length(); i++) {
		output[i] = nearestInteger(Sc[i], w);
	}
	return output;
}

const mat_ZZ keySwitchMatrix(const mat_ZZ& S, const mat_ZZ& T) {
	mat_ZZ Sstar = getBitMatrix(S);
	mat_ZZ A = getRandomMatrix(T.NumCols(),Sstar.NumCols(),aBound);
	mat_ZZ E = getRandomMatrix(Sstar.NumRows(),Sstar.NumCols(),eBound);
	return vCat(Sstar + E - T*A, A);
}

const vec_ZZ encrypt(const mat_ZZ& T, const vec_ZZ& x) {
	mat_ZZ I;
	ident(I, x.length());
	return keySwitch(keySwitchMatrix(I, T), w * x);
}




const vec_ZZ addVectors(const vec_ZZ& c1, const vec_ZZ& c2){
	return c1 + c2;
}

const vec_ZZ linearTransform(const mat_ZZ& M, const vec_ZZ& c){
	return M * getBitVector(c);
}

const mat_ZZ linearTransformClient(const mat_ZZ& G, const mat_ZZ& S, const mat_ZZ& T){
	return keySwitchMatrix(G * S, T);
}


const vec_ZZ innerProd(const vec_ZZ& c1, const vec_ZZ& c2, const mat_ZZ& M){
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

const mat_ZZ innerProdClient(const mat_ZZ& T){
	mat_ZZ S = getSecretKey(T);
	mat_ZZ tvsts = transpose(vectorize(transpose(S) * S));
	mat_ZZ mvsts = copyRows(tvsts, T.NumRows());
	return keySwitchMatrix(mvsts, T);
}




const mat_ZZ copyRows(const mat_ZZ& row, long numrows){
	mat_ZZ ans;
	ans.SetDims(numrows, row.NumCols());
	for (int i=0; i<ans.NumRows(); ++i){
		for (int j=0; j<ans.NumCols(); ++j){
			ans[i][j] = row[0][j];
		}
	}
	return ans;
}

const mat_ZZ vectorize(const mat_ZZ& M){
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
	ifstream cin("vhe.in");

	cin.tie(NULL);
	ios_base::sync_with_stdio(false);

	stack<vec_ZZ*> vectors;
	stack<mat_ZZ*> matrices;

	string operation;
	while (cin >> operation) {
		//cerr << "Operation: " << operation << endl;

		if (operation == "vector") {
			vec_ZZ* v = new vec_ZZ();
			cin >> (*v);
			//cerr << "Vector (" << v.length() << ")" << endl;
			vectors.push(v);

		} else if (operation == "matrix") {
			mat_ZZ* m = new mat_ZZ();
			cin >> (*m);
			//cerr << "Matrix (" << m.NumRows() << ", " << m.NumCols() << ")" << endl;
			matrices.push(m);

		} else if (operation == "duplicate-vector") {
			vectors.push(vectors.top());

		} else if (operation == "duplicate-matrix") {
			matrices.push(matrices.top());

		} else if (operation == "add") {
			vec_ZZ& v1 = *vectors.top(); vectors.pop();
			vec_ZZ& v2 = *vectors.top(); vectors.pop();
			vec_ZZ* v = new vec_ZZ();
			(*v) = addVectors(v1, v2);
			vectors.push(v);

		} else if (operation == "scalar-multiply") {
			ZZ x;
			cin >> x;
			vec_ZZ& v = *vectors.top(); vectors.pop();
			vec_ZZ* v2 = new vec_ZZ();
			(*v2) = v * x;
			vectors.push(v2);

		} else if (operation == "linear-transform") {
			vec_ZZ& v = *vectors.top(); vectors.pop();
			mat_ZZ& m = *matrices.top(); matrices.pop();
			vec_ZZ* r = new vec_ZZ();
			(*r) = linearTransform(m, v);
			vectors.push(r);

		} else if (operation == "linear-transform-key-switch") {
			mat_ZZ& T = *matrices.top(); matrices.pop();
			mat_ZZ& S = *matrices.top(); matrices.pop();
			mat_ZZ& G = *matrices.top(); matrices.pop();
			mat_ZZ* m = new mat_ZZ();
			(*m) = linearTransformClient(G, S, T);
			matrices.push(m);

		} else if (operation == "inner-product") {
			vec_ZZ& v1 = *vectors.top(); vectors.pop();
			vec_ZZ& v2 = *vectors.top(); vectors.pop();
			mat_ZZ& m = *matrices.top(); matrices.pop();
			vec_ZZ *v = new vec_ZZ();
			(*v) = innerProd(v1, v2, m);
			vectors.push(v);

		} else if (operation == "inner-product-key-switch") {
			mat_ZZ& T = *matrices.top(); matrices.pop();
			mat_ZZ *m = new mat_ZZ();
			(*m) = innerProdClient(T);
			matrices.push(m);

		} else if (operation == "key-switch") {
			vec_ZZ& v = *vectors.top(); vectors.pop();
			mat_ZZ& m = *matrices.top(); matrices.pop();
			vec_ZZ *v2 = new vec_ZZ();
			(*v2) = keySwitch(m, v);
			vectors.push(v2);

		} else if (operation == "random-matrix") {
			int rows, cols;
			cin >> rows >> cols;
			mat_ZZ *m = new mat_ZZ();
			(*m) = getRandomMatrix(rows, cols, tBound);
			matrices.push(m);

		} else if (operation == "identity") {
			int rows;
			cin >> rows;
			mat_ZZ *I = new mat_ZZ();
			ident(*I, rows);
			matrices.push(I);

		} else if (operation == "key-switch-matrix") {
			mat_ZZ& T = *matrices.top(); matrices.pop();
			mat_ZZ& S = *matrices.top(); matrices.pop();
			mat_ZZ *m = new mat_ZZ();
			(*m) = keySwitchMatrix(S, T);
			matrices.push(m);

		} else if (operation == "get-secret-key") {
			mat_ZZ &T = *matrices.top(); matrices.pop();
			mat_ZZ *s = new mat_ZZ();
			(*s) = getSecretKey(T);
			matrices.push(s);

		} else if (operation == "encrypt") {
			mat_ZZ& T = *matrices.top(); matrices.pop();
			vec_ZZ& x = *vectors.top(); vectors.pop();
			vec_ZZ *v = new vec_ZZ();
			(*v) = encrypt(T, x);
			vectors.push(v);

		} else if (operation == "decrypt") {
			mat_ZZ& S = *matrices.top(); matrices.pop();
			vec_ZZ& c = *vectors.top(); vectors.pop();
			vec_ZZ *x = new vec_ZZ();
			(*x) = decrypt(S, c);
			vectors.push(x);

		} else {
			cerr << "Unknown command: " << operation << endl;
		}

	}

	stack<vec_ZZ> vectors2;
	while (vectors.size()) {
		vectors2.push(*vectors.top()); vectors.pop();
	}
	while (vectors2.size()) {
		cout << vectors2.top() << endl; vectors2.pop();
	}

	stack<mat_ZZ> matrices2;
	while (matrices.size()) {
		matrices2.push(*matrices.top()); matrices.pop();
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

