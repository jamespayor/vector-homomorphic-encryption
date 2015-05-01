#include <iostream>
#include <sstream>
#include <cassert>
#include <NTL/mat_ZZ_p.h>
#include <NTL/mat_ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <NTL/vec_ZZ_p.h>
#include <NTL/vec_ZZ.h>
#include <cmath>
#include <stack>

using namespace std;
using namespace NTL;

const ZZ w(134503000), q = w * w * w * w * w;
const ZZ aBound(1000), eBound(100);
const int l = 140;

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
vec_ZZ_p linearTransform(mat_ZZ_p M, vec_ZZ_p c);

// returns M, the key switch matrix from GS to S,
// to be sent to server
mat_ZZ_p linearTransformClient(mat_ZZ_p T, mat_ZZ_p G);

//
vec_ZZ_p inprod(vec_ZZ_p c1, vec_ZZ_p c2, ZZ w, mat_ZZ_p M);

// returns M, the key switch matrix from vec(S^t S) to S,
// to be sent to the server
mat_ZZ_p inprodClient(mat_ZZ_p T);

// returns a column vector
mat_ZZ_p vectorize(mat_ZZ_p M);





mat_ZZ tozz(mat_ZZ_p m);
mat_ZZ_p tozzp(mat_ZZ m);




mat_ZZ tozz(mat_ZZ_p m){
    mat_ZZ A;
    A.SetDims(m.NumRows(), m.NumCols());
    for (int i=0; i<m.NumRows(); ++i){
        for (int j=0; j<m.NumCols(); ++j){
            A[i][j] = conv<ZZ>(m[i][j]);
        }
    }
    return A;
}

mat_ZZ_p tozzp(mat_ZZ m){
    mat_ZZ_p A;
    A.SetDims(m.NumRows(), m.NumCols());
    for (int i=0; i<m.NumRows(); ++i){
        for (int j=0; j<m.NumCols(); ++j){
            A[i][j] = conv<ZZ_p>(m[i][j]);
        }
    }
    return A;
}







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




vec_ZZ_p addVectors(vec_ZZ_p c1, vec_ZZ_p c2){
    return c1 + c2;
}

vec_ZZ_p linearTransform(mat_ZZ_p M, vec_ZZ_p c){
    return M * getBitVector(c);
}

mat_ZZ_p linearTransformClient(mat_ZZ_p T, mat_ZZ_p G){
    mat_ZZ_p I;
    ident(I, T.NumRows());
    mat_ZZ_p S = hCat(I, T);
    return keySwitchMatrix(G * S, T, aBound, eBound);
}


vec_ZZ_p innerProd(vec_ZZ_p c1, vec_ZZ_p c2, mat_ZZ_p M){
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

mat_ZZ_p innerProdClient(mat_ZZ_p T){
    mat_ZZ_p I;
    ident(I, T.NumRows());
    mat_ZZ_p S = hCat(I, T);
    mat_ZZ_p vectorizedSTransposeS = vectorize(transpose(S) * S);
    return keySwitchMatrix(vectorizedSTransposeS, T, aBound, eBound);
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
/*
    stack<vec_ZZ_p> vectors;
    stack<mat_ZZ_p> matrices;

    string operation;
    while (cin >> operation) {

        if (operation == "vector") {
            vec_ZZ_p v;
            cin >> v;
            vectors.push(v);

        } else if (operation == "matrix") {
            mat_ZZ_p m;
            cin >> m;
            matrices.push(m);

        } else if (operation == "duplicate-vector") {
            vectors.push(vectors.top());

        } else if (operation == "duplicate-matrix") {
            matrices.push(matrices.top());

        } else if (operation == "add") {
            vec_ZZ_p v1 = vectors.top(); vectors.pop();
            vec_ZZ_p v2 = vectors.top(); vectors.pop();
            vectors.push(addVectors(v1, v2));

        } else if (operation == "scalar-multiply") {
            ZZ_p x;
            cin >> x;
            vec_ZZ_p v = vectors.top(); vectors.pop();
            vectors.push(v * x);

        } else if (operation == "linear-transform") {
            vec_ZZ_p v = vectors.top(); vectors.pop();
            mat_ZZ_p m = matrices.top(); matrices.pop();
            vectors.push(linearTransform(m, v));

        } else if (operation == "inner-product") {
            vec_ZZ_p v1 = vectors.top(); vectors.pop();
            vec_ZZ_p v2 = vectors.top(); vectors.pop();
            mat_ZZ_p m = matrices.top(); matrices.pop();
            vectors.push(innerProd(v1, v2, m));

        }

    }

    while (vectors.size()) {
        cout << vectors.top(); vectors.pop();
    }
*/


    const int N = 30;
	vec_ZZ x1;
	vec_ZZ x2;
	x1.SetLength(N);
	x2.SetLength(N);
	for(int i = 0; i < N; ++i) {
		x1[i] = RandomBnd(10000);
		x2[i] = RandomBnd(10000);
	}
	cout << x1 << endl;
	cout << x2 << endl;
	mat_ZZ_p T = getRandomMatrix(N, N, aBound);
	vec_ZZ_p c1 = encrypt(T, x1);
	vec_ZZ_p c2 = encrypt(T, x2);


//// testing for addition:
//	vec_ZZ_p cplus;
//	vec_ZZ dxplus;
//	vec_ZZ xplus;
//
//	cplus = addn(c1, c2);
//	dxplus = decrypt(getSecretKey(T), cplus);
//	xplus = x1 + x2;
//
//	cout << xplus << endl;
//	cout << dxplus << endl;
//  cout << xplus - dxplus << endl;


//// testing for linear transform
//    mat_ZZ_p G;
//    mat_ZZ_p M;
//    vec_ZZ_p cc;
//    vec_ZZ dxx;
//    vec_ZZ xx;
//
//    G = getRandomMatrix(N, N, aBound);
//    M = linearTransformClient(T, G);
//    cc = linearTransform(M, c1);
//    dxx = decrypt(getSecretKey(T), cc);
//    xx = tozz(G) * x1;
//
//	cout << G << endl;
//	cout << xx << endl;
//	cout << dxx << endl;
//	cout << xx - dxx << endl;


}

