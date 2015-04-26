#include <iostream>
#include <sstream>
#include <cassert>
#include <NTL/mat_ZZ_p.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <NTL/vec_ZZ_p.h>
#include <NTL/vec_ZZ.h>
#include <cmath>
#include <stack>


using namespace std;
using namespace NTL;

const ZZ w(134503000), q = w * w * w * w;
const ZZ aBound(12345), eBound(0);
const int l = 100;

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


mat_ZZ_p getRandomMatrix(long row, long col, ZZ bound);




// server side addition with same secret key
vec_ZZ_p addn(vec_ZZ_p c1, vec_ZZ_p c2);

// server side linear transformation,
// returns S(Gx) given c=Sx and M (key switch matrix from GS to S)
vec_ZZ_p lntr(mat_ZZ_p M, vec_ZZ_p c);

// returns M, the key switch matrix from GS to S,
// to be sent to server
mat_ZZ_p lntrClient(mat_ZZ_p T, mat_ZZ_p G);

//
vec_ZZ_p inprod(vec_ZZ_p c1, vec_ZZ_p c2, ZZ w, mat_ZZ_p M);

// returns M, the key switch matrix from vec(S^t S) to S,
// to be sent to the server
mat_ZZ_p inprodClient(mat_ZZ_p T);

// returns a column vector
mat_ZZ_p vectorize(mat_ZZ_p M);


// finds c* then returns Mc*
vec_ZZ_p keySwitch(mat_ZZ_p M, vec_ZZ_p c){
    vec_ZZ_p cstar = getBitVector(c);
    return M * cstar;
}


mat_ZZ_p getRandomMatrix(long row, long col, ZZ bound){
    mat_ZZ_p A;
    A.SetDims(row, col);
    for (int i=0; i<row; ++i){
        for (int j=0; j<col; ++j){
            A[i][j] = conv<ZZ_p>(RandomBnd(bound));
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
		output[i] = (rep(Sc[i])+(w+1)/2)/w;
	}
	return output;
}

mat_ZZ_p keySwitchMatrix(mat_ZZ_p S, mat_ZZ_p T, ZZ Abound, ZZ Ebound) {
	mat_ZZ_p Sstar = getBitMatrix(S);
	mat_ZZ_p A = getRandomMatrix(T.NumCols(),Sstar.NumCols(),Abound);
	mat_ZZ_p E = getRandomMatrix(Sstar.NumRows(),Sstar.NumCols(),Ebound);
	mat_ZZ_p M = vCat(Sstar + E - T*A, A);
	return M;
}

vec_ZZ_p encrypt(mat_ZZ_p T, vec_ZZ x) {
	mat_ZZ_p I;
	ident(I, x.length());
	return keySwitch(keySwitchMatrix(I, T, aBound, eBound), conv<vec_ZZ_p>(w * x));
}




vec_ZZ_p addn(vec_ZZ_p c1, vec_ZZ_p c2){
    return c1 + c2;
}

vec_ZZ_p lntr(mat_ZZ_p M, vec_ZZ_p c){
    return M * c;
}

mat_ZZ_p lntrClient(mat_ZZ_p T, mat_ZZ_p G){
	mat_ZZ_p I;
	ident(I, T.NumRows());
    mat_ZZ_p S = hCat(I, T);
    return keySwitchMatrix(G * S, T, aBound, eBound);
}


vec_ZZ_p inprod(vec_ZZ_p c1, vec_ZZ_p c2, ZZ w, mat_ZZ_p M){
    mat_ZZ_p cc1;
    mat_ZZ_p cc2;
    mat_ZZ_p cc;

    cc1.SetDims(c1.length(), 1);
    for (int i=0; i<c1.length(); ++i){
        cc1[i][0] = c1[i];
    }
    cc2.SetDims(1, c2.length());
    for (int i=0; i<c2.length(); ++i){
        cc1[0][i] = c2[i];
    }
    cc = vectorize(cc1 * cc2);

    vec_ZZ_p output;
	output.SetLength(cc.NumRows());
	for (int i=0; i<cc.NumRows(); i++) {
		output[i] = conv<ZZ_p>((rep(cc[i][0])+(w+1)/2)/w);
	}
    return M * output;
}

mat_ZZ_p inprodClient(mat_ZZ_p T){
	mat_ZZ_p I;
	ident(I, T.NumRows());
    mat_ZZ_p S = hCat(I, T);
    mat_ZZ_p vsts;

    vsts = vectorize(transpose(S) * S);
    return keySwitchMatrix(vsts, T, aBound, eBound);
}


mat_ZZ_p vectorize(mat_ZZ_p M){
    mat_ZZ_p ans;
    ans.SetDims(M.NumRows() * M.NumCols(), 1);
    for (int i=0; i<M.NumRows(); ++i){
        for (int j=0; j<M.NumCols(); ++j){
            ans[i*M.NumCols() + j][0] = M[i][j];
        }
    }
    return ans;
}















int main()
{
	ZZ_p::init(q);

	string line;
	stack<vec_ZZ_p> vectors;

	while(getline(cin, line)) {
		if (line[0] == '+') {
			vec_ZZ_p c1 = vectors.top(); vectors.pop();
			vec_ZZ_p c2 = vectors.top(); vectors.pop();
			vectors.push(c1 + c2);
		} else if (line[0] == '*') {
			vec_ZZ_p c1 = vectors.top(); vectors.pop();
			vec_ZZ_p c2 = vectors.top(); vectors.pop();
			ZZ_p prod = c1 * c2;
			vec_ZZ_p c;
			c.SetLength(1);
			c[0] = prod;
			vectors.push(c);
		} else {
			stringstream stream(line);
			vec_ZZ_p c;
			stream >> c;
			vectors.push(c);
		}
	}

	while(vectors.size()) {
		cout << vectors.top() << endl;
		vectors.pop();
	}

	/*
	vec_ZZ d;
	const int N = 30;
	d.SetLength(N);
	for(int i = 0; i < N; ++i) {
		d[i] = RandomBnd(10000);
	}

	mat_ZZ_p T = getRandomMatrix(d.length(), d.length(), aBound);

	vec_ZZ_p c = encrypt(T, d);

	vec_ZZ x = decrypt(getSecretKey(T), c);

	cout << d << endl;
	cout << x << endl;
	*/
}

