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

const ZZ w_zz(1345030000L);
ZZ_p w;
const ZZ q = w_zz * w_zz * w_zz * w_zz * w_zz;
ZZ_p aBound, eBound;
const int l = 70;

vec_ZZ_p decrypt(mat_ZZ_p S, vec_ZZ_p c);

mat_ZZ_p hCat(mat_ZZ_p A, mat_ZZ_p B);
mat_ZZ_p vCat(mat_ZZ_p A, mat_ZZ_p B);

// returns c*
vec_ZZ_p getBitVector(vec_ZZ_p c);

// returns S*
mat_ZZ_p getBitMatrix(mat_ZZ_p S);

// returns S
mat_ZZ_p getSecretKey(mat_ZZ_p T);

// returns M
mat_ZZ_p keySwitchMatrix(mat_ZZ_p S, mat_ZZ_p T, ZZ_p Abound, ZZ_p Ebound);

// finds c* then returns Mc*
vec_ZZ_p keySwitch(mat_ZZ_p M, vec_ZZ_p c);

// as described, treating I as the secret key and wx as ciphertext
vec_ZZ_p encrypt(mat_ZZ_p T, vec_ZZ_p x);


mat_ZZ_p getRandomMatrix(long row, long col, ZZ_p bound);




// server side addition with same secret key
vec_ZZ_p addn(vec_ZZ_p c1, vec_ZZ_p c2);

// server side linear transformation,
// returns S(Gx) given c=Sx and M (key switch matrix from GS to S)
vec_ZZ_p linearTransform(mat_ZZ_p M, vec_ZZ_p c);

// returns M, the key switch matrix from GS to S,
// to be sent to server
mat_ZZ_p linearTransformClient(mat_ZZ_p T, mat_ZZ_p G);

// computes an inner product, given two ciphertexts and the keyswitch matrix
vec_ZZ_p innerProd(vec_ZZ_p c1, vec_ZZ_p c2, mat_ZZ_p M);

// returns M, the key switch matrix from vec(S^t S) to S,
// to be sent to the server
mat_ZZ_p innerProdClient(mat_ZZ_p T);


// computes an inner product, given two ciphertexts and the keyswitch matrix
vec_ZZ_p innerProdNoSwitch(vec_ZZ_p c1, vec_ZZ_p c2);

// returns M, the key switch matrix from vec(S^t S) to S,
// to be sent to the server
vec_ZZ_p innerProdNoSwitchDecrypt(vec_ZZ_p cc, mat_ZZ_p T);


// returns a column vector
mat_ZZ_p vectorize(mat_ZZ_p M);


mat_ZZ_p copyRows(mat_ZZ_p row, long numrows);


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


mat_ZZ_p getRandomMatrix(long row, long col, ZZ_p bound){
    mat_ZZ_p A;
    A.SetDims(row, col);
    for (int i=0; i<row; ++i){
        for (int j=0; j<col; ++j){
            A[i][j] = conv<ZZ_p>(RandomBnd(conv<ZZ>(bound)));
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


vec_ZZ_p decrypt(mat_ZZ_p S, vec_ZZ_p c) {
    vec_ZZ_p Sc = S*c;
    vec_ZZ_p output;
    output.SetLength(Sc.length());
    for (int i=0; i<Sc.length(); i++) {
        output[i] = conv<ZZ_p>((rep(Sc[i])+(rep(w)+1)/2)/rep(w));
    }
    return output;
}

mat_ZZ_p keySwitchMatrix(mat_ZZ_p S, mat_ZZ_p T, ZZ_p Abound, ZZ_p Ebound) {
    mat_ZZ_p Sstar = getBitMatrix(S);
    mat_ZZ_p A = getRandomMatrix(T.NumCols(),Sstar.NumCols(),Abound);
    mat_ZZ_p E = getRandomMatrix(Sstar.NumRows(),Sstar.NumCols(),Ebound);
    mat_ZZ_p M = vCat(Sstar + E - T*A, A);
    return M;
}

vec_ZZ_p encrypt(mat_ZZ_p T, vec_ZZ_p x) {
    mat_ZZ_p I;
    ident(I, x.length());
    return keySwitch(keySwitchMatrix(I, T, aBound, eBound), w * x);
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
        cc2[0][i] = c2[i];
    }
    cc = vectorize(cc1 * cc2);

    vec_ZZ_p output;
    output.SetLength(cc.NumRows());
    for (int i=0; i<cc.NumRows(); i++) {
        output[i] = conv<ZZ_p>((rep(cc[i][0])+(rep(w)+1)/2)/rep(w));
    }
    return M * getBitVector(output);
}

mat_ZZ_p innerProdClient(mat_ZZ_p T){
    mat_ZZ_p I;
    ident(I, T.NumRows());
    mat_ZZ_p S = hCat(I, T);
    mat_ZZ_p tvsts = transpose(vectorize(transpose(S) * S));
    mat_ZZ_p mvsts = copyRows(tvsts, T.NumRows());
    return keySwitchMatrix(mvsts, T, aBound, eBound);
}




vec_ZZ_p innerProdNoSwitch(vec_ZZ_p c1, vec_ZZ_p c2){
    mat_ZZ_p cc1;
    mat_ZZ_p cc2;
    mat_ZZ_p cc;

    cc1.SetDims(c1.length(), 1);
    for (int i=0; i<c1.length(); ++i){
        cc1[i][0] = c1[i];
    }
    cc2.SetDims(1, c2.length());
    for (int i=0; i<c2.length(); ++i){
        cc2[0][i] = c2[i];
    }
    cc = vectorize(cc1 * cc2);

    vec_ZZ_p output;
    output.SetLength(cc.NumRows());
    for (int i=0; i<cc.NumRows(); i++) {
        output[i] = conv<ZZ_p>((rep(cc[i][0])+(rep(w)+1)/2)/rep(w));
    }
    return output;
}

vec_ZZ_p innerProdNoSwitchDecrypt(vec_ZZ_p cc, mat_ZZ_p T){
    mat_ZZ_p I;
    ident(I, T.NumRows());
    mat_ZZ_p S = hCat(I, T);
    mat_ZZ_p tvsts = transpose(vectorize(transpose(S) * S));
    return decrypt(tvsts, cc);
}




mat_ZZ_p copyRows(mat_ZZ_p row, long numrows){
    mat_ZZ_p ans;
    ans.SetDims(numrows, row.NumCols());
    for (int i=0; i<ans.NumRows(); ++i){
        for (int j=0; j<ans.NumCols(); ++j){
            ans[i][j] = row[0][j];
        }
    }
    return ans;
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
    w = conv<ZZ_p>(w_zz);
    aBound = 1000;
    eBound = 5;

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

        } else if (operation == "linear-transform-key-switch") {
            mat_ZZ_p T = matrices.top(); matrices.pop();
            mat_ZZ_p G = matrices.top(); matrices.pop();
            matrices.push(linearTransformClient(T, G));

        } else if (operation == "inner-product") {
            vec_ZZ_p v1 = vectors.top(); vectors.pop();
            vec_ZZ_p v2 = vectors.top(); vectors.pop();
            mat_ZZ_p m = matrices.top(); matrices.pop();
            vectors.push(innerProd(v1, v2, m));

        } else if (operation == "inner-product-key-switch") {
            mat_ZZ_p T = matrices.top(); matrices.pop();
            matrices.push(innerProdClient(T));

        } else if (operation == "key-switch") {
            vec_ZZ_p v = vectors.top(); vectors.pop();
            mat_ZZ_p m = matrices.top(); matrices.pop();
            vectors.push(keySwitch(m, v));

        } else if (operation == "random-matrix") {
            int dimension;
            cin >> dimension;
            matrices.push(getRandomMatrix(dimension, dimension, aBound));

        } else if (operation == "identity") {
            int rows;
            cin >> rows;
            mat_ZZ_p I;
            ident(I, rows);
            matrices.push(I);

        } else if (operation == "key-switch-matrix") {
            mat_ZZ_p T = matrices.top(); matrices.pop();
            mat_ZZ_p S = matrices.top(); matrices.pop();
            matrices.push(keySwitchMatrix(S, T, aBound, eBound));

        } else if (operation == "get-secret-key") {
            mat_ZZ_p T = matrices.top(); matrices.pop();
            matrices.push(getSecretKey(T));

        } else if (operation == "encrypt") {
            mat_ZZ_p T = matrices.top(); matrices.pop();
            vec_ZZ_p x = vectors.top(); vectors.pop();
            vectors.push(encrypt(T, x));

        } else if (operation == "decrypt") {
            mat_ZZ_p S = matrices.top(); matrices.pop();
            vec_ZZ_p c = vectors.top(); vectors.pop();
            vectors.push(decrypt(S, c));

        } else {
            cerr << "Unknown command: " << operation << endl;
        }

    }

    stack<vec_ZZ_p> vectors2;
    while (vectors.size()) {
        vectors2.push(vectors.top()); vectors.pop();
    }
    while (vectors2.size()) {
        cout << vectors2.top() << endl; vectors2.pop();
    }

    stack<mat_ZZ_p> matrices2;
    while (matrices.size()) {
        matrices2.push(matrices.top()); matrices.pop();
    }
    while (matrices2.size()) {
        cout << matrices2.top() << endl; matrices2.pop();
    }


//// Testing for the 3 fundamental operations:
//    const int N = 40;
//	vec_ZZ_p x1;
//	vec_ZZ_p x2;
//	x1.SetLength(N);
//	x2.SetLength(N);
//	for(int i = 0; i < N; ++i) {
//		x1[i] = RandomBnd(10000);
//		x2[i] = RandomBnd(10000);
//	}
//	cout << x1 << endl;
//	cout << x2 << endl;
//	mat_ZZ_p T = getRandomMatrix(N, 1, aBound);
//	vec_ZZ_p c1 = encrypt(T, x1);
//	vec_ZZ_p c2 = encrypt(T, x2);
//
//
//// testing for inner product no switch
//
//    vec_ZZ_p cc = innerProdNoSwitch(c1, c2);
//    vec_ZZ_p dxx = innerProdNoSwitchDecrypt(cc, T);
//    ZZ_p xx;
//    InnerProduct(xx, x1, x2);
//
//    cout << xx << endl;
//    cout << dxx[0] << endl;
//    cout << xx - dxx[0] << endl;


//// testing for inner product
//    mat_ZZ_p M;
//    vec_ZZ_p cc;
//    vec_ZZ_p dxx;
//    ZZ_p xx;
//
//    M = innerProdClient(T);
//    cc = innerProd(c1, c2, M);
//    dxx = decrypt(getSecretKey(T), cc);
//    InnerProduct(xx, x1, x2);
//
//
//    cout << xx << endl;
//    cout << dxx[0] << endl;
//    cout << xx - dxx[0] << endl;




//// testing for linear transform
//    mat_ZZ_p G;
//    mat_ZZ_p M;
//    vec_ZZ_p cc;
//    vec_ZZ_p dxx;
//    vec_ZZ_p xx;
//
//    G = getRandomMatrix(N, N, aBound);
//    M = linearTransformClient(T, G);
//    cc = linearTransform(M, c1);
//    dxx = decrypt(getSecretKey(T), cc);
//    xx = G * x1;
//
//	cout << G << endl;
//	cout << xx << endl;
//	cout << dxx << endl;
//	cout << xx - dxx << endl;



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


}

