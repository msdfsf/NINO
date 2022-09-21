#pragma once

#define DFT_MTX_MAX_LEN 8192

namespace dsplib {

	typedef struct fComplex {

		double r;
		double i;

	} fComplex;

	// pre-computed DFT_MATRIX of N DFT_MTX_MAX_LEN with double precision
	//extern const float DFT_MTX[];

	// pre-allocate needed array of len <= DFT_MTX_MAX_LEN && len = 2 ^ n, where n is 1, 2 ... etc.
	// and use this function to fill it
	void getDFTMatrix(fComplex* expWn, int n);

	void fftFloat(double* signalIn, double* signalOut, const fComplex* expWn, const int n);

}
