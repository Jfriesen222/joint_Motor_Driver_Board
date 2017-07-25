#include <math.h>
#include <matrix.h>
#include <mex.h>
#include <iostream>
#include <sstream>
#include <string>
#include "QuadProg++.hh"

void print_matrix2(char* name, const Matrix<double>& A, int n = -1, int m = -1);

/* The gateway function */
void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[])
{
    double *H;               /* H>>G   nxn*/
    double *f;               /* f>>g0  1xn input matrix */
    double *A;               /* A>>CI  nxp input matrix */
    double *b;               /* b>>ci0 1xp input matrix */
    size_t nn,pp;              /* size of matrix */
    double *w;               /* output  vector */
    
    /* check for proper number of arguments */
    if(nrhs!=4) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:nrhs","Four inputs required.");
    }
    if(nlhs!=1) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:nlhs","One output required.");
    }
    
    /* make sure the input arguments are type double */
    if( !mxIsDouble(prhs[0]) ||
            mxIsComplex(prhs[0])||!mxIsDouble(prhs[1]) ||
            mxIsComplex(prhs[1])||!mxIsDouble(prhs[2]) ||
            mxIsComplex(prhs[2])||!mxIsDouble(prhs[3]) ||
            mxIsComplex(prhs[3])) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:notDouble","Input matrix must be type double.");
    }
    
    /* create pointers to the real data in the input matrix  */
    H = mxGetPr(prhs[0]);
    f = mxGetPr(prhs[1]);
    A = mxGetPr(prhs[2]);
    b = mxGetPr(prhs[3]);
    
    /* get dimensions of the input matrix */
    nn = mxGetN(prhs[0]);
    pp = mxGetM(prhs[3]);
    
    /* create the output matrix */
    plhs[0] = mxCreateDoubleMatrix((mwSize)nn,1,mxREAL);
    
    /* get a pointer to the real data in the output matrix */
    w = mxGetPr(plhs[0]);
    
//     //declare variables
//     mxArray *a_in_m, *b_in_m, *c_out_m, *d_out_m;
//     const mwSize *dims;
//     double *a, *b, *c, *d;
//     int dimx, dimy, numdims;
//
//     //associate inputs
//     a_in_m = mxDuplicateArray(prhs[0]);
//     b_in_m = mxDuplicateArray(prhs[1]);
//
//     //figure out dimensions
//     dims = mxGetDimensions(prhs[0]);
//     numdims = mxGetNumberOfDimensions(prhs[0]);
//     dimy = (int)dims[0]; dimx = (int)dims[1];
//
//     //associate outputs
//     c_out_m = plhs[0] = mxCreateDoubleMatrix(dimy,dimx,mxREAL);
//     d_out_m = plhs[1] = mxCreateDoubleMatrix(dimy,dimx,mxREAL);
    
    
    Matrix<double> G, CE, CI;
    Vector<double> g0, ce0, ci0, x;
    int  n,m,p;
    double sum = 0.0;
    char ch;
    n = (int) nn;
    p = (int) pp;
    m = 0;
    G.resize(n, n);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            G[i][j] = H[n*i+j];
    //print_matrix2("G", G);
    
    g0.resize(n);
		for (int i = 0; i < n; i++)
			g0[i] = f[i];


    CE.resize(n, m);
    ce0.resize(m);
   
    CI.resize(n, p);
		for (int i = 0; i < n; i++)
			for (int j = 0; j < p; j++)
				CI[i][j] = -A[p*i+j];
    //print_matrix2("CI", CI);
    ci0.resize(p);
		for (int j = 0; j < p; j++)
			ci0[j] = b[j];

    x.resize(n);
    double ff;
    ff = solve_quadprog(G, g0, CE, ce0, CI, ci0, x);
    for (int j = 0; j < n; j++)
			w[j] = x[j];
    /*  for (int i = 0; i < n; i++)
     * std::cout << x[i] << ' ';
     * std::cout << std::endl;	 */
    
    /* FOR DOUBLE CHECKING COST since in the solve_quadprog routine the matrix G is modified */
    
// 	{
// 		std::istringstream is("4, -2,"
// 			"-2, 4 ");
//
// 		for (int i = 0; i < n; i++)
// 			for (int j = 0; j < n; j++)
// 				is >> G[i][j] >> ch;
// 	}
//
// 	std::cout << "Double checking cost: ";
// 	for (int i = 0; i < n; i++)
// 		for (int j = 0; j < n; j++)
// 			sum += x[i] * G[i][j] * x[j];
// 	sum *= 0.5;
//
// 	for (int i = 0; i < n; i++)
// 		sum += g0[i] * x[i];
// 	std::cout << sum << std::endl;
    /*  for (int i = 0; i < n; i++)
     * std::cout << x[i] << ' ';
     * std::cout << std::endl;	 */
    
    /* FOR DOUBLE CHECKING COST since in the solve_quadprog routine the matrix G is modified */
    
}

void print_matrix2(char* name, const Matrix<double>& A, int n, int m)
{
    if (n == -1)
        n = A.nrows();
    if (m == -1)
        m = A.ncols();
    
    mexPrintf("%s: ",name);
    for (int i = 0; i < m; i++)
    {
        mexPrintf(" ");
        for (int j = 0; j < n; j++)
            mexPrintf("%f, ", A[j][i]);
        mexPrintf("\r\n");
    }
}

void print_vector2(char* name, const Vector<double>& A, int n, int m)
{
//     if (n == -1)
//         n = A.nrows();
//     
//     mexPrintf("%s: ",name);
//     for (int i = 0; i < m; i++)
//     {
//         mexPrintf(" ");
//         for (int j = 0; j < n; j++)
//             mexPrintf("%f, ", A[j][i]);
//         mexPrintf("\r\n");
//     }
}