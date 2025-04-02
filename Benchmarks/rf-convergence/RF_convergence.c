//
//  RF_convergence.c
//
//  Created by Matthew Kinsinger (Student) on 2/20/20.
//
//  Combining already written applications of sync_MMSE_beamformer (Matt), STAP_Comms (Matt),
//  and temporal_mitigation (Saquib) to make a full (simplistic) RF convergence simulation
//  in which both a radar and communications signal are overlapping in time and frequency.
//  We sync to the comms signal, MMSE estimate, decode and remodulate, then temporally project
//  the full received data onto the subspace orthogonal to the received comms signal.
//
//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
//#include<iostream>
//#include<fstream>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_interp.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_matrix_complex_double.h>
#include <math.h>

#include "dash.h"
#include <gsl/gsl_fft_complex.h>

#define PROGPATH  "./data/"
#define RX PROGPATH "rx.dat"
#define PDPULSE PROGPATH "input_pd_pulse.txt"
//#define DATAEST PROGPATH "dataEstimate.txt"
#define XTRAIN PROGPATH "x_Training.dat"
#define SYNCTRAIN PROGPATH "sync_symbols.dat"

// Prototypes of the functions to be called by main()
void gsl_fft(double *input_array, double *output_array, size_t n_elements) {
	// allocate wavetable and workspace for this length FFT
	gsl_fft_complex_wavetable *wavetable = gsl_fft_complex_wavetable_alloc(n_elements);
	gsl_fft_complex_workspace *workspace = gsl_fft_complex_workspace_alloc(n_elements);

	// since this is an in-place transform, copy the input data to the output array
	for (size_t i = 0; i < 2 * n_elements; i++) {
		output_array[i] = input_array[i];
	}

	// do the transform
	gsl_fft_complex_forward(output_array, 1, n_elements, wavetable, workspace);

	// deallocate wavetable and workspace
	gsl_fft_complex_wavetable_free(wavetable);
	gsl_fft_complex_workspace_free(workspace);
}

void gsl_ifft(double *input_array, double *output_array, size_t n_elements) {
	// allocate wavetable and workspace for this length FFT
	gsl_fft_complex_wavetable *wavetable = gsl_fft_complex_wavetable_alloc(n_elements);
	gsl_fft_complex_workspace *workspace = gsl_fft_complex_workspace_alloc(n_elements);

	// since this is an in-place transform, copy the input data to the output array
	for (size_t i = 0; i < 2 * n_elements; i++) {
		output_array[i] = input_array[i];
	}

	// do the transform
	gsl_fft_complex_inverse(output_array, 1, n_elements, wavetable, workspace);

	// deallocate wavetable and workspace
	gsl_fft_complex_wavetable_free(wavetable);
	gsl_fft_complex_workspace_free(workspace);
}
// returns the starting time sample index for the comms signal (ipeak = 400)

void LU_factorization_projection(gsl_matrix_complex *Z_temp_proj, int numRx, gsl_matrix_complex *S_temp_delay,gsl_matrix_complex *S_temp_delay_cpy,
                                 int Ntaps_projection, int modulo_N);
void xcorr(double *x, double *y, size_t n_samp, double *corr);

void ConjTrans(gsl_matrix_complex **in)
{
	gsl_matrix_complex *input = *in;

	gsl_matrix_complex *temp = gsl_matrix_complex_alloc(input->size2, input->size1);
	int rows = input->size1;
	// printf("inside rows = %d \n", rows);
    int cols = input->size2;
	// printf("inside cols = %d \n", cols);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            gsl_complex z = gsl_matrix_complex_get(input, i, j);
            gsl_complex z_conj = gsl_complex_conjugate(z);
            gsl_matrix_complex_set(temp, j, i, z_conj);
        }
    }

	// gsl_matrix_complex_free(*in);

	*in = temp;
}

int main() {
	// Read in data structures and parameters
	int numRx = 4;  // ALWAYS KEEP AS 4!!!!!!!!!!!!!! (to be compatible with Saquib's temporal mitigation code)
	int numTx = 1;  // ALWAYS KEEP AS 1!!!!!!!!!!!!!! (to be compatible with Saquib's temporal mitigation code)
	int Ntaps = 5;  // for sync and MMSE beamforming
	int initial_offset = -1;   // begin at the -1th delay tap for the STAP data matrix
	int Ntaps_projection = 4;  // ALWAYS KEEP AS 4!!!!!!!!!!!!!! (to be compatible with Saquib's temporal mitigation
	                           // code)
	// error checking
	if (numRx != Ntaps_projection) {
		printf(
		    "Number of receive antennas needs to be equal to Ntaps for the projection matrix (both equal to 4).\r\n");
		printf("numRx: %d, Ntaps_projection: %d\r\n", numRx, Ntaps_projection);
		exit(4);
	}
	int modulo_N = 64;        // We need to call Saquib's temporal mitigation code in blocks of 64 time samples
	// Reduce the number of blocks to speed up computation
	int n_sync_blocks = 4;    // number of comms sync blocks of 64 time samples (originally 4)
	int n_trn_blocks = 4;     // number of comms training blocks of 64 time samples (originally 4)
	int n_data_blocks = 10; // number of comms data blocks of 64 time samples (originally 10)
	int n_start_zeros = 400;
	int num_sync_samp = n_sync_blocks * modulo_N;
	int n_sync_trn_zeros = 100;  // zero padding between comms sync and training signals
	int num_trn_samp = n_trn_blocks * modulo_N;
	int num_trn_data_zeros = 100;  // zero padding between comms training and data signals
	int num_data_samp = n_data_blocks * modulo_N;

	int n_data_end_zeros = 18;  // zero padding at the end of the comms signal.

	// Calculate total samples based on the (reduced) block counts and padding
	int samp_sum = n_start_zeros + num_sync_samp + n_sync_trn_zeros + num_trn_samp + num_trn_data_zeros +
	               num_data_samp + n_data_end_zeros;
	int num_rx_samp = samp_sum; // Total number of samples is now smaller
	// The previous check comparing samp_sum and a hardcoded num_rx_samp is removed.

	// if (samp_sum != num_rx_samp) {
	// 	printf("Number of total Rx time samples is not correct?\r\n");
	// 	printf("Sum of samples: %d, N Rx samples: %d\r\n", samp_sum, num_rx_samp);
	// 	exit(4);
	// }
	// QAM comms constellation parameters (hard coded for now)
	int N_constellation_points = 4;
	gsl_matrix_complex *QAM_constellation = NULL;
	QAM_constellation = gsl_matrix_complex_alloc(1, N_constellation_points);
	gsl_matrix_complex_set(QAM_constellation, 0, 0, gsl_complex_rect(1., 0.));
	gsl_matrix_complex_set(QAM_constellation, 0, 1, gsl_complex_rect(0., 1.));
	gsl_matrix_complex_set(QAM_constellation, 0, 2, gsl_complex_rect(-1., 0.));
	gsl_matrix_complex_set(QAM_constellation, 0, 3, gsl_complex_rect(0., -1.));

	/*
	 * Read in raw data - and form into signal matrix
	 */
	gsl_matrix_complex *rxSig = NULL;
	FILE *rawDataFile = fopen(RX, "r");
	// FILE *rawDataFile = fopen(RX,"r");
	rxSig = gsl_matrix_complex_alloc(numRx, num_rx_samp);
	double temp_real, temp_imag;
	gsl_complex temp_complex = gsl_complex_rect(1., 0.);
	int dataNumPulses;
	fread(&dataNumPulses, sizeof(int), 1, rawDataFile);
	// if ((numRx * num_rx_samp) != dataNumPulses) {
	// 	printf(
	// 	    "Number of pulses specified by config file and raw data file differ! Are you sure this data matches your config?\r\n");
	// 	printf("dataNumPulses: %d, numPulses: %d\r\n", dataNumPulses, numRx * num_rx_samp);
	// 	exit(4);
	// }
	//KERN_ENTER(make_label("fileLoad[2D][%d,%d][complex][float64]",numRx,num_rx_samp));
    for (int i = 0; i < num_rx_samp; i++) {
		for (int j = 0; j < numRx; j++) {
			fread(&temp_real, sizeof(double), 1, rawDataFile);
			fread(&temp_imag, sizeof(double), 1, rawDataFile);
			temp_complex = gsl_complex_rect(temp_real, temp_imag);
			gsl_matrix_complex_set(rxSig, j, i, temp_complex);
		}
	}
	//KERN_EXIT(make_label("fileLoad[2D][%d,%d][complex][float64]",numRx,num_rx_samp));
	fclose(rawDataFile);

	//     //debugging: check to see if we read the data in correctly (done)
	//     {
	//         FILE * f = fopen ("readInRxSig.txt", "w");
	//         gsl_matrix_complex_fprintf(f,rxSig,"%f");
	//         fclose(f);
	//     }

	printf("Done reading in received signal.\r\n");

	/*
	 * Read in sync data
	 */
	gsl_matrix_complex *sync_symbols = NULL;
	FILE *rawDataFile2 = fopen(SYNCTRAIN, "r");
	// FILE *rawDataFile2 = fopen(SYNCTRAIN,"r");
	sync_symbols = gsl_matrix_complex_alloc(numTx, num_sync_samp);
	fread(&dataNumPulses, sizeof(int), 1, rawDataFile2);
	// Commenting out check: File size might not match reduced num_sync_samp
	// if ((num_sync_samp) != dataNumPulses) {
	//	printf(
	//	    "Number of pulses specified by config file and raw data file differ! Are you sure this data matches your config?\r\n");
	//	printf("dataNumPulses: %d, numPulses: %d\r\n", dataNumPulses, num_sync_samp);
	//	exit(4);
	//}
    //KERN_ENTER(make_label("fileLoad[1D][%d][complex][float64]",num_sync_samp));
	for (int i = 0; i < 1; i++) {
		for (int j = 0; j < num_sync_samp; j++) {
			fread(&temp_real, sizeof(double), 1, rawDataFile2);
			fread(&temp_imag, sizeof(double), 1, rawDataFile2);
			temp_complex = gsl_complex_rect(temp_real, temp_imag);
			gsl_matrix_complex_set(sync_symbols, i, j, temp_complex);
		}
	}
    //KERN_EXIT(make_label("fileLoad[1D][%d][complex][float64]",num_sync_samp));
	fclose(rawDataFile2);
	printf("Done reading in sync reference.\r\n");

	//     //debugging: check to see if we read the data in correctly (done)
	//     {
	//         FILE * f = fopen ("readInX_training.txt", "w");
	//         gsl_matrix_complex_fprintf(f,x_Training,"%f");
	//         fclose(f);
	//     }

	/*
	 * Read in comms training data
	 */
	gsl_matrix_complex *x_Training = NULL;
	FILE *rawDataFile3 = fopen(XTRAIN, "r");
	// FILE *rawDataFile3 = fopen(XTRAIN,"r");
	x_Training = gsl_matrix_complex_alloc(1, num_trn_samp);
	fread(&dataNumPulses, sizeof(int), 1, rawDataFile3);
	// Commenting out check: File size might not match reduced num_trn_samp
	// if ((num_trn_samp) != dataNumPulses) {
	//	printf(
	//	    "Number of pulses specified by config file and raw data file differ! Are you sure this data matches your config?\r\n");
	//	printf("dataNumPulses: %d, numPulses: %d\r\n", dataNumPulses, num_trn_samp);
	//	exit(4);
	//}
    //KERN_ENTER(make_label("fileLoad[1D][%d][complex][float64]",num_trn_samp));
	for (int i = 0; i < 1; i++) {
		for (int j = 0; j < num_trn_samp; j++) {
			// NOTE: Reading from rawDataFile2 here might be a bug in the original code?
			// Should likely be rawDataFile3. Correcting this.
			fread(&temp_real, sizeof(double), 1, rawDataFile3); // Changed rawDataFile2 to rawDataFile3
			fread(&temp_imag, sizeof(double), 1, rawDataFile3); // Changed rawDataFile2 to rawDataFile3
			temp_complex = gsl_complex_rect(temp_real, temp_imag);
			gsl_matrix_complex_set(x_Training, i, j, temp_complex);
		}
	}
    //KERN_EXIT(make_label("fileLoad[1D][%d][complex][float64]",num_trn_samp));
	fclose(rawDataFile3); // Changed rawDataFile2 to rawDataFile3
	printf("Done reading in training reference.\r\n");

	//     //debugging: check to see if we read the data in correctly (done)
	//     {
	//         FILE * f = fopen ("readInX_training.txt", "w");
	//         gsl_matrix_complex_fprintf(f,x_Training,"%f");
	//         fclose(f);
	//     }

	// =================== begin synchronization to the comms signal ======================================
	// Get real and imaginary parts of the complex matrices (both sync and rx data)
	// Dynamic memory initialization
	double *sync_symbols_real, *sync_symbols_imag;
	double *rx_data_sync_real, *rx_data_sync_imag;
	// Dynamic memory allocation
	sync_symbols_real = (double *)malloc(numTx * num_sync_samp * sizeof(double));
	sync_symbols_imag = (double *)malloc(numTx * num_sync_samp * sizeof(double));

	rx_data_sync_real = (double *)malloc(numRx * num_rx_samp * sizeof(double));
	rx_data_sync_imag = (double *)malloc(numRx * num_rx_samp * sizeof(double));

	for (int i = 0; i < numTx; i++) {
		for (int j = 0; j < num_sync_samp; j++) {
			// QUESTION: Does this dynamic memory access automatically account for the size of the data type?
			sync_symbols_real[i * num_sync_samp + j] = GSL_REAL(gsl_matrix_complex_get(sync_symbols, i, j));
			sync_symbols_imag[i * num_sync_samp + j] = GSL_IMAG(gsl_matrix_complex_get(sync_symbols, i, j));
		}
	}

	for (int i = 0; i < numRx; i++) {
		for (int j = 0; j < num_rx_samp; j++) {
			rx_data_sync_real[i * num_rx_samp + j] = GSL_REAL(gsl_matrix_complex_get(rxSig, i, j));
			rx_data_sync_imag[i * num_rx_samp + j] = GSL_IMAG(gsl_matrix_complex_get(rxSig, i, j));
		}
	}


	int ipeak = 400;
	printf("ipeak: %d\n", ipeak);

	// =================== end sync to the comms signal ======================================

	// =================== begin MMSE comms signal estimation ================================

	// beginning at the first comms training sample we build the Space-time receive matrix
	int comms_sync_start_idx = ipeak;
	int comms_trn_start_idx = ipeak + num_sync_samp + n_sync_trn_zeros;
	int comms_data_start_idx = comms_trn_start_idx + num_trn_samp + num_trn_data_zeros;

	// Make the delayed matrix, rxSigDelays (-1 + LOS + 3 delay taps, say)
	int stackedMatSize = Ntaps * numRx;
	double zero = 0;
	gsl_complex complexZero = gsl_complex_rect(zero, zero);
	// Slide rows of rxSig matrix to the left by 1 sample. Fill in with zeros at the end
	gsl_matrix_complex *rxSigDelays = NULL;
	rxSigDelays = gsl_matrix_complex_alloc(stackedMatSize, num_trn_samp);  // allocate memory for the space-time matrix
	// fill in the space-time matrix using rxSig. Zero pad at the end.
	for (int i = 0; i < numRx; i++) {             // antenna loop
		for (int j = 0; j < num_trn_samp; j++) {  // time samples loop
			// place the samples for antenna i for LOS and all delays
			for (int k = initial_offset; k < Ntaps + initial_offset; k++) {  // delay taps loop
				// printf("I: %d, J: %d, K: %d\r\n",i,j,k);
				if (j + k <= num_trn_samp - 1 + initial_offset) {
					temp_complex = gsl_matrix_complex_get(rxSig, i, comms_trn_start_idx + j + k);  // the k-th delay
					                                                                               // sample
					gsl_matrix_complex_set(rxSigDelays, numRx * (k - initial_offset) + i, j, temp_complex);
				} else {
					gsl_matrix_complex_set(rxSigDelays, numRx * (k - initial_offset) + i, j, complexZero);  // set	                                                                                        // 0+i0
				}
			}
		}
	}

	gsl_matrix_complex *autoCorrMatrix = NULL;
	autoCorrMatrix = gsl_matrix_complex_alloc(stackedMatSize, stackedMatSize);  // allocate memory
	gsl_complex sampleToConjugate = gsl_complex_rect(1., 0.);  // To be used to hold the sample to be conjugate in each
	                                                           // loop
	gsl_complex dot_prod = gsl_complex_rect(0., 0.);           // declare type for the dot product accumulator
	//KERN_ENTER(make_label("GEMM[Ar-%d][Ac-%d][Bc-%d][complex][float64]",stackedMatSize,num_trn_samp,stackedMatSize));
	for (int i = 0; i < stackedMatSize; i++) {
		for (int k = 0; k < stackedMatSize; k++) {

			dot_prod = gsl_complex_rect(0., 0.);
			for (int j = 0; j < num_trn_samp; j++) {
				temp_complex = gsl_matrix_complex_get(rxSigDelays, i, j);
				sampleToConjugate = gsl_matrix_complex_get(rxSigDelays, k, j);  // This is the sample to conjugate
				sampleToConjugate = gsl_complex_conjugate(sampleToConjugate);
				temp_complex = gsl_complex_mul(temp_complex, sampleToConjugate);  // Multiply the two samples
				dot_prod = gsl_complex_add(dot_prod, temp_complex);               // accumulate (dot product)
			}
			// Place this dot product into the cross correlation matrix
			gsl_matrix_complex_set(autoCorrMatrix, i, k, dot_prod);
		}
	}
	//KERN_EXIT(make_label("GEMM[Ar-%d][Ac-%d][Bc-%d][complex][float64]",stackedMatSize,num_trn_samp,stackedMatSize));
	// end making the auto-correlation matrix

	//     //debugging: check to see if we buit auto-correlation matrix correctly (done)
	//     {
	//         FILE * f = fopen ("autoCorrMatrix.txt", "w");
	//         gsl_matrix_complex_fprintf(f,autoCorrMatrix,"%f");
	//         fclose(f);
	//     }

	// Invert the auto-correlation matrix
	gsl_permutation *p = gsl_permutation_alloc(stackedMatSize);
	int s;
	// Compute the LU decomposition of this matrix
	printf("just before LU decomposition\n");

	//KERN_ENTER(make_label("matrixInverse[Ar-%d][Ac-%d]",stackedMatSize,stackedMatSize));
	gsl_linalg_complex_LU_decomp(autoCorrMatrix, p, &s);
	// Compute the  inverse of the LU decomposition
	gsl_matrix_complex *invAutoCorr = gsl_matrix_complex_alloc(stackedMatSize, stackedMatSize);
	printf("just before inversion\n");
	gsl_linalg_complex_LU_invert(autoCorrMatrix, p, invAutoCorr);
	//KERN_EXIT(make_label("matrixInverse[Ar-%d][Ac-%d]",stackedMatSize,stackedMatSize));
	
	printf("just after inversion\n");
	gsl_permutation_free(p);

	//     //debugging: check to see if we inverted the auto-correlation matrix correctly (done)
	//     {
	//         FILE * f = fopen ("invAutoCorr.txt", "w");
	//         gsl_matrix_complex_fprintf(f,invAutoCorr,"%f");
	//         fclose(f);
	//     }

	// Matrix multiply arrayResponse = rxSigDelayed * x_training^H
	gsl_matrix_complex *arrayResponse = NULL;
	arrayResponse = gsl_matrix_complex_alloc(stackedMatSize, 1);  // allocate memory
	// complex matrix multiplication
	gsl_complex unity = gsl_complex_rect(1., 0.);  // the complex number 1
	printf("just before making array response.\n");

	//KERN_ENTER(make_label("GEMM[Ar-%d][Ac-%d][Bc-1][complex][float64]",stackedMatSize,num_trn_samp));
	KernelEnter("GEMM");
	gsl_blas_zgemm(CblasNoTrans, CblasConjTrans, unity, rxSigDelays, x_Training, complexZero, arrayResponse);
	KernelExit("GEMM");
	//KERN_EXIT(make_label("GEMM[Ar-%d][Ac-%d][Bc-1][complex][float64]",stackedMatSize,num_trn_samp));

	//     //debugging: check to see if we built the array response vector correctly (done)
	//     {
	//         FILE * f = fopen ("arrayResponse.txt", "w");
	//         gsl_matrix_complex_fprintf(f,arrayResponse,"%f");
	//         fclose(f);
	//     }

	printf("just before making beamormer.\n");
	// Matrix mulitpy  (R*R^Hermitian)^-1 * (arrayResponse)
	// This gives us our beamforming vector w (20x1)
	gsl_matrix_complex *beamFormer = NULL;
	beamFormer = gsl_matrix_complex_alloc(stackedMatSize, 1);  // allocate memory
	// complex matrix multiplication

	//KERN_ENTER(make_label("GEMM[Ar-%d][Ac-%d][Bc-1][complex][float64]",stackedMatSize,stackedMatSize));
	KernelEnter("GEMM");
	gsl_blas_zgemm(CblasNoTrans, CblasNoTrans, unity, invAutoCorr, arrayResponse, complexZero, beamFormer);
	KernelExit("GEMM");
	//KERN_EXIT(make_label("GEMM[Ar-%d][Ac-%d][Bc-1][complex][float64]",stackedMatSize,stackedMatSize));

	printf("just after making beamormer.\n");

	//     //debugging: check to see if we built the beamformer vector correctly (done)
	//     {
	//         FILE * f = fopen ("beamFormer.txt", "w");
	//         gsl_matrix_complex_fprintf(f,beamFormer,"%f");
	//         fclose(f);
	//     }

	// Make the delayed matrix, rxSigDelays (LOS + 4 delay taps, say)
	// Slide rows of rxSig matrix to the left by 1 sample. Fill in with zeros at the end

	int Nextra = -initial_offset;  // we still need to make sure we grab all the way to the end of the data sequence
	                               // length
	gsl_matrix_complex *rxDataDelays = NULL;
	rxDataDelays = gsl_matrix_complex_alloc(stackedMatSize, num_data_samp + Nextra);  // allocate memory for the
	                                                                                  // space-time matrix
	// fill in the space-time matrix using rxSig
	for (int i = 0; i < numRx; i++) {                       // antenna loop
		for (int j = 0; j < num_data_samp + Nextra; j++) {  // time samples loop
			// place the samples for antenna i for LOS and all delays
			for (int k = initial_offset; k < Ntaps + initial_offset; k++) {  // delay taps loop beginning at -1
				//               printf("I: %d, J: %d, K: %d\r\n",i,j,k);
				if (j + k < num_data_samp - 1 + initial_offset + Nextra) {
					temp_complex = gsl_matrix_complex_get(rxSig, i, comms_data_start_idx + j + k);  // the k-th delay
					                                                                                // sample
					gsl_matrix_complex_set(rxDataDelays, numRx * (k - initial_offset) + i, j, temp_complex);
				} else {
					gsl_matrix_complex_set(rxDataDelays, numRx * (k - initial_offset) + i, j, complexZero);  // set
					                                                                                         // samples
					                                                                                         // at the
					                                                                                         // end to
					                                                                                         // 0+i0
				}
			}
		}
	}

	printf("Built delayed data matrix.\r\n");

	// Multiply w^H * rxDataDelayed
	gsl_matrix_complex *dataEstimate = NULL;
	dataEstimate = gsl_matrix_complex_alloc(1, num_data_samp + Nextra);  // allocate memory
	// complex matrix multiplication
	//KERN_ENTER(make_label("GEMM[Ar-1][Ac-%d][Bc-%d][complex][float64]",stackedMatSize,num_data_samp + Nextra));
	KernelEnter("GEMM");
	gsl_blas_zgemm(CblasConjTrans, CblasNoTrans, unity, beamFormer, rxDataDelays, complexZero, dataEstimate);
	KernelExit("GEMM");
	//KERN_EXIT(make_label("GEMM[Ar-1][Ac-%d][Bc-%d][complex][float64]",stackedMatSize,num_data_samp + Nextra));

	// MAKE SURE TO DISREGARD THE LAST <Nextra> ENTRY'S OF <dataEstimate> IN ANY FURTHER PROCESSING (i.e. when building
	// the projection matrix)

	printf("Formed beamformed data estimate.\r\n");
	// This gives us our estimate of the transmitted signal (1xnumDataSamples?)
	//     // Write out estimated signal to file.
	//     {
	//         FILE * f = fopen ("data_estimate.txt", "w");
	//         gsl_matrix_complex_fprintf(f,dataEstimate,"%f");
	//         fclose(f);
	//     }

	// Now need to map the estimate of what was transmitted back to QAM symbols by
	// finding the QAM symbol that is nearest to the estimated symbol
	// QAM_constellation 1x4
	// dataEstimate 1xnum_data_samp    (make sure to disregard the last Nextra (=1) entries

	// gsl_matrix_complex *remodulated_symbols = NULL;
	// remodulated_symbols = gsl_matrix_complex_alloc(1,num_data_samp);

	// make a vector to temporarily hold the distance metrics
	double *temp_dist = (double *)malloc(N_constellation_points * sizeof(double));
	int min_idx;
	gsl_complex temp_complex2 = gsl_complex_rect(1., 0.);

	//KERN_ENTER(make_label("ZIP[4QAM_argmin][%d][float64][complex]",num_data_samp*N_constellation_points));
	for (int i = 0; i < num_data_samp; i++) {
		temp_complex = gsl_matrix_complex_get(dataEstimate, 0, i);
		for (int j = 0; j < N_constellation_points; j++) {
			temp_complex2 = gsl_matrix_complex_get(QAM_constellation, 0, j);
			temp_complex2 = gsl_complex_sub(temp_complex, temp_complex2);
			temp_dist[j] = gsl_complex_abs2(temp_complex2);
		}
		// find minimum entry in temp_dist
		min_idx = 0;
		for (int k = 1; k < N_constellation_points; k++) {
			if (temp_dist[k] < temp_dist[min_idx]) {
				min_idx = k;
			}
		}
		// quanitze the dataEstimate to the nearest QAM symbol
		gsl_matrix_complex_set(dataEstimate, 0, i, gsl_matrix_complex_get(QAM_constellation, 0, min_idx));
	}
	//KERN_EXIT(make_label("ZIP[4QAM_argmin][%d][float64][complex]",num_data_samp*N_constellation_points));

	printf("Remodulated the data estimate.\r\n");

	// =================== end MMSE comms signal estimation ================================

	// =================== begin orthogonal projection =====================================
	// Now we need to project the received signal onto the subspace orthogonal to the comms signal.
	// Beginning at the sync index we project the sync signal, trn signal, and comms data signal.
	// IGNORE the sections between these signal where we zero padded the comms signal.

	//=============== BEGIN sync symbols section =========================================

	// Allocate the array of pointers first
	gsl_matrix_complex **S_temp_delay = malloc(sizeof(gsl_matrix_complex*) * n_sync_blocks);
	gsl_matrix_complex **S_temp_delay_cpy = malloc(sizeof(gsl_matrix_complex*) * n_sync_blocks);
	gsl_matrix_complex **Z_temp_proj = malloc(sizeof(gsl_matrix_complex*) * n_sync_blocks);
	// int* helper_array = malloc(sizeof(int) * 10);
	// helper_array[0] = 0;

	// Allocate each individual matrix and perform initialization
	for (int k = 0; k < n_sync_blocks; k++) {
		// Allocate the actual matrix for this index
		S_temp_delay[k] = gsl_matrix_complex_alloc(numTx * Ntaps_projection, modulo_N); // 4x64
		S_temp_delay_cpy[k] = gsl_matrix_complex_alloc(numTx * Ntaps_projection, modulo_N); // 4x64
		Z_temp_proj[k] = gsl_matrix_complex_alloc(numRx, modulo_N);                 // 4x64

		
		// Initialize specific elements for S_temp_delay[i]
		// It's generally safer to zero out first if the subsequent loops don't fill everything
		gsl_matrix_complex_set_zero(S_temp_delay[k]); 
		gsl_matrix_complex_set(S_temp_delay[k], 0, modulo_N - 1, complexZero);  // always the case for all blocks
		gsl_matrix_complex_set(S_temp_delay[k], 2, 0, complexZero);             // always the case for all blocks
		gsl_matrix_complex_set(S_temp_delay[k], 3, 0, complexZero);             // always the case for all blocks
		gsl_matrix_complex_set(S_temp_delay[k], 3, 1, complexZero);             // always the case for all blocks

		gsl_matrix_complex_set_zero(S_temp_delay_cpy[k]); 
		gsl_matrix_complex_set(S_temp_delay_cpy[k], 0, modulo_N - 1, complexZero);  // always the case for all blocks
		gsl_matrix_complex_set(S_temp_delay_cpy[k], 2, 0, complexZero);             // always the case for all blocks
		gsl_matrix_complex_set(S_temp_delay_cpy[k], 3, 0, complexZero);             // always the case for all blocks
		gsl_matrix_complex_set(S_temp_delay_cpy[k], 3, 1, complexZero);             // always the case for all blocks

		for (int i = 0; i < modulo_N - 1; i++) {
			gsl_matrix_complex_set(S_temp_delay[k], 0, i, gsl_matrix_complex_get(sync_symbols, 0, k * modulo_N + i + 1));
			gsl_matrix_complex_set(S_temp_delay_cpy[k], 0, i, gsl_matrix_complex_get(sync_symbols, 0, k * modulo_N + i + 1));
		}
		// Now the LOS and other delayed taps
		for (int i = 1; i < Ntaps_projection; i++) {
			for (int j = 0; j < modulo_N - (i - 1); j++) {
				gsl_matrix_complex_set(S_temp_delay[k], i, j + (i - 1),
				                       gsl_matrix_complex_get(sync_symbols, 0, k * modulo_N + j));
				gsl_matrix_complex_set(S_temp_delay_cpy[k], i, j + (i - 1),
				                       gsl_matrix_complex_get(sync_symbols, 0, k * modulo_N + j));
			}
		}
		// Now grab the relevant received data block
		for (int i = 0; i < numRx; i++) {
			for (int j = 0; j < modulo_N; j++) {
				gsl_matrix_complex_set(Z_temp_proj[k], i, j,
				                       gsl_matrix_complex_get(rxSig, i, comms_sync_start_idx + k * modulo_N + j));
			}
		}
		
	}
	printf("Finished initializing S_temp_delay and Z_temp_proj.\n");

	// Now the non-zero entries (one block of 64 samples at a time)
	// The main processing loop remains largely the same, using [k] index
	for (int k = 0; k < n_sync_blocks; k++) {
		NonKernelSplit();
		printf("Finished NonKernelSplit.\n");
		// The -1_th delay tap
		// Note: The S_temp_delay[k] initialization for specific zero elements was moved to the allocation loop above.
		// Ensure the loops below correctly populate the non-zero elements without relying on prior state other than the zeros just set.
		
		// Now project using this block. After calling the function the matrix Z_temp_proj will hold the receivd data
		// after projection onto the subspace orthogonal to S_temp_delay.
		LU_factorization_projection(Z_temp_proj[k], 4, S_temp_delay[k],S_temp_delay_cpy[k], 4, 64);
		// Replace the corresponding section of the received data with this newly projected data
		// for (int i = 0; i < numRx; i++) {
		// 	for (int j = 0; j < modulo_N; j++) {
		// 		gsl_matrix_complex_set(rxSig, i, comms_sync_start_idx + k * modulo_N + j,
		// 		                       gsl_matrix_complex_get(Z_temp_proj[k], i, j));
		// 	}
		// }
		// helper_array[0] = 2;
		NonKernelSplit();
	}


	// printf("Starting training symbols section.helper_array[0] = %d\n", helper_array[0]);
	// gsl_matrix_complex **S_temp_delay1 = malloc(sizeof(gsl_matrix_complex*) * n_sync_blocks);
	// gsl_matrix_complex **Z_temp_proj1 = malloc(sizeof(gsl_matrix_complex*) * n_sync_blocks);
	// gsl_matrix_complex ** rxSig_res = &rxSig;
	// for (int k = 0; k < n_trn_blocks; k++) {
	// 	// Allocate the actual matrix for this index
	// 	S_temp_delay1[k] = gsl_matrix_complex_alloc(numTx * Ntaps_projection, modulo_N); // 4x64
	// 	Z_temp_proj1[k] = gsl_matrix_complex_alloc(numRx, modulo_N);                 // 4x64

	// 	gsl_matrix_complex_set_zero(S_temp_delay1[k]); 
	// 	gsl_matrix_complex_set(S_temp_delay1[k], 0, modulo_N - 1, complexZero);  // always the case for all blocks
	// 	gsl_matrix_complex_set(S_temp_delay1[k], 2, 0, complexZero);             // always the case for all blocks
	// 	gsl_matrix_complex_set(S_temp_delay1[k], 3, 0, complexZero);             // always the case for all blocks
	// 	gsl_matrix_complex_set(S_temp_delay1[k], 3, 1, complexZero);             // always the case for all blocks

	// 	for (int i = 0; i < modulo_N - 1; i++) {
	// 		gsl_matrix_complex_set(S_temp_delay1[k], 0, i, gsl_matrix_complex_get(x_Training, 0, k * modulo_N + i + 1));
	// 	}
	// 	// Now the LOS and other delayed taps
	// 	for (int i = 1; i < Ntaps_projection; i++) {
	// 		for (int j = 0; j < modulo_N - (i - 1); j++) {
	// 			gsl_matrix_complex_set(S_temp_delay1[k], i, j + (i - 1),
	// 			                       gsl_matrix_complex_get(x_Training, 0, k * modulo_N + j));
	// 		}
	// 	}
	// 	// Now grab the relevant received data block
	// 	for (int i = 0; i < numRx; i++) {
	// 		for (int j = 0; j < modulo_N; j++) {
	// 			// int comms_trn_start_idx = ipeak + num_sync_samp + n_sync_trn_zeros;
	// 			gsl_matrix_complex_set(Z_temp_proj1[k], i, j,
	// 			                       gsl_matrix_complex_get(*rxSig_res, i, comms_trn_start_idx + k * modulo_N + j));
	// 		}
	// 	}
	// }


	// for (int k = 0; k < n_trn_blocks; k++) {
	// 	NonKernelSplit();

	// 	// The -1_th delay tap
		
	// 	// Now project using this block. After calling the function the matrix Z_temp_proj will hold the receivd data
	// 	// after projection onto the subspace orthogonal to S_temp_delay.
	// 	LU_factorization_projection(Z_temp_proj1[k], numRx, S_temp_delay1[k], Ntaps_projection, modulo_N);
	// 	// Replace the corresponding section of the received data with this newly projected data
	// 	for (int i = 0; i < numRx; i++) {
	// 		for (int j = 0; j < modulo_N; j++) {
	// 			// int comms_sync_start_idx = ipeak;
	// 			gsl_matrix_complex_set(rxSig, i, comms_trn_start_idx + k * modulo_N + j,
	// 			                       gsl_matrix_complex_get(Z_temp_proj1[k], i, j));
	// 		}
	// 	}
	// 	NonKernelSplit();
	// }
	// // // //=============== END training symbols section =========================================

	// // // //=============== BEGIN data symbols section =========================================

	// // // // Now the non-zero entries (one block of 64 samples at a time)
	// // // for (int k = 0; k < n_data_blocks; k++) {
	// // // 	// The -1_th delay tap
	// // // 	for (int i = 0; i < modulo_N - 1; i++) {
	// // // 		gsl_matrix_complex_set(S_temp_delay, 0, i, gsl_matrix_complex_get(dataEstimate, 0, k * modulo_N + i + 1));
	// // // 	}
	// // // 	// Now the LOS and other delayed taps
	// // // 	for (int i = 1; i < Ntaps_projection; i++) {
	// // // 		for (int j = 0; j < modulo_N - (i - 1); j++) {
	// // // 			gsl_matrix_complex_set(S_temp_delay, i, j + (i - 1),
	// // // 			                       gsl_matrix_complex_get(dataEstimate, 0, k * modulo_N + j));
	// // // 		}
	// // // 	}
	// // // 	// Now grab the relevant received data block
	// // // 	for (int i = 0; i < numRx; i++) {
	// // // 		for (int j = 0; j < modulo_N; j++) {
	// // // 			// int comms_data_start_idx = comms_trn_start_idx + num_trn_samp + num_trn_data_zeros;
	// // // 			gsl_matrix_complex_set(Z_temp_proj, i, j,
	// // // 			                       gsl_matrix_complex_get(rxSig, i, comms_data_start_idx + k * modulo_N + j));
	// // // 		}
	// // // 	}
	// // // 	// Now project using this block. After calling the function the matrix Z_temp_proj will hold the receivd data
	// // // 	// after projection onto the subspace orthogonal to S_temp_delay.
	// // // 	LU_factorization_projection(Z_temp_proj, numRx, S_temp_delay, Ntaps_projection, modulo_N);
	// // // 	// Replace the corresponding section of the received data with this newly projected data
	// // // 	for (int i = 0; i < numRx; i++) {
	// // // 		for (int j = 0; j < modulo_N; j++) {
	// // // 			// int comms_sync_start_idx = ipeak;
	// // // 			gsl_matrix_complex_set(rxSig, i, comms_data_start_idx + k * modulo_N + j,
	// // // 			                       gsl_matrix_complex_get(Z_temp_proj, i, j));
	// // // 		}
	// // // 	}
	// // // }
	// // // //=============== END data symbols section =========================================

	// // // printf("Done projecting onto subspace orthogonal to comms signal.");
	// // // // =================== END orthogonal projection =====================================

	// // // //=============== BEGIN radar processing section =========================================
	
	// // // // Original value was 1024. This might exceed the new, smaller num_rx_samp.
	// // // // Cap n_samples at num_rx_samp to avoid out-of-bounds access.
	// // // size_t n_samples = 1024; 
	// // // if (n_samples > num_rx_samp) {
	// // // 	n_samples = num_rx_samp; // Use the available samples if less than 1024
	// // // 	printf("Warning: Radar processing using reduced n_samples = %zu due to smaller num_rx_samp.\n", n_samples);
	// // // }
	

	
	// // // double B = 1e6;
	// // // double T = 100/B;
	// // // double sampling_rate = 1e6;
	// // // double lag;
	// // // // Allocate based on the potentially adjusted n_samples
	// // // double *corr = malloc((2 * (2 * n_samples - 1)) * sizeof(double)); 
	// // // double *received = malloc(2 * n_samples * sizeof(double));
	// // // double *pulse = malloc(2 * n_samples * sizeof(double));         // array for the original pulse
	

	// // // // This loop now correctly uses n_samples which is <= num_rx_samp
	// // // for (int i = 0; i < n_samples; i++) {
	// // // 	temp_complex = gsl_complex_add(gsl_matrix_complex_get(rxSig, 0, i),gsl_complex_add(gsl_matrix_complex_get(rxSig, 1, i), gsl_complex_add(gsl_matrix_complex_get(rxSig, 2, i),gsl_matrix_complex_get(rxSig, 3, i))));
	// // // 	received[2*i] = GSL_REAL(temp_complex);
	// // // 	received[2*i + 1] = GSL_IMAG(temp_complex);
	// // // }

    // // FILE *fp;
	// // fp = fopen(PDPULSE, "r");  // read the original pulse
	// // // Ensure we don't read past the end of the file if it's smaller than expected,
	// // // and don't read more than allocated for pulse.
	// // for (int i = 0; i < 2 * n_samples; i++) {
	// // 	fscanf(fp, "%lf", &pulse[i]);
	// // }
	// // fclose(fp);
	

	// // xcorr(received, pulse, n_samples, corr);

	// // Code to find maximum
	// double max_corr = 0,tmp=0;
	// double index = 0;
	// // Adjust loop bounds for corr array size
	// for (size_t i = 0; i < 2 * (2 * n_samples - 1); i += 2) { 
	// 	// Only finding maximum of real part of correlation
	// 	tmp = corr[i]*corr[i] + corr[i+1]*corr[i+1];
	// 	// Original code only checked real part for maximum. Keeping that logic.
	// 	if (corr[i] > max_corr) { 
	// 		max_corr = corr[i];
	// 		index = i / 2;
	// 	}
	// }
	
	// lag = ((2 * n_samples) - 1 - index) / sampling_rate;
	
	// printf("Lag Value is: %lf\n", lag); // Added newline for cleaner output

	// // Free allocated memory at the end
	// free(corr);
	// free(received);
	// free(pulse);
	// gsl_matrix_complex_free(rxSig);
	// gsl_matrix_complex_free(sync_symbols);
	// gsl_matrix_complex_free(x_Training);
	// gsl_matrix_complex_free(QAM_constellation);
	// free(sync_symbols_real);
	// free(sync_symbols_imag);
	// free(rx_data_sync_real);
	// free(rx_data_sync_imag);
	// gsl_matrix_complex_free(rxSigDelays);
	// gsl_matrix_complex_free(autoCorrMatrix);
	// gsl_matrix_complex_free(invAutoCorr);
	// gsl_matrix_complex_free(arrayResponse);
	// gsl_matrix_complex_free(beamFormer);
	// gsl_matrix_complex_free(rxDataDelays);
	// gsl_matrix_complex_free(dataEstimate);
	// gsl_matrix_complex_free(S_temp_delay);
	// gsl_matrix_complex_free(Z_temp_proj);


	return 0;
	// END RF_convergence.c
}


void TransToDashGemm(CBLAS_TRANSPOSE_t TransA, CBLAS_TRANSPOSE_t TransB, const gsl_complex alpha, const gsl_matrix_complex *A, const gsl_matrix_complex *B, const gsl_complex beta, gsl_matrix_complex *C)
{
	if (TransA==CblasConjTrans) {
		ConjTrans(&A);
		// printf("outside rows = %d \n", A->size1);
		// printf("outside cols = %d \n", A->size2);

	}

	if (TransB==CblasConjTrans) {
		ConjTrans(&B);
		// printf("outside rows = %d \n", B->size1);
		// printf("outside cols = %d \n", B->size2);
	}

	double *A_real, *A_imag;
	double *B_real, *B_imag;
	double *C_real, *C_imag;
	// Dynamic memory allocation
	A_real = (double *)malloc(A->size1 * A->size2 * sizeof(double));
	A_imag = (double *)malloc(A->size1 * A->size2 * sizeof(double));

	B_real = (double *)malloc(B->size1 * B->size2 * sizeof(double));
	B_imag = (double *)malloc(B->size1 * B->size2 * sizeof(double));

	C_real = (double *)malloc(C->size1 * C->size2 * sizeof(double));
	C_imag = (double *)malloc(C->size1 * C->size2 * sizeof(double));
	
	for (int i = 0; i < A->size1; i++) {
		for (int j = 0; j < A->size2; j++) {
			A_real[i * A->size2 + j] = 0;
			A_imag[i * A->size2 + j] = 0;
		}
	}

	for(int i = 0; i < B->size1; i++) {
		for(int j = 0; j < B->size2; j++) {
			B_real[i * B->size2 + j] = 0;
			B_imag[i * B->size2 + j] = 0;
		}
	}

	
	// this if statement causes problem because the loop induction variables are not known at compile time, they are struct not variables. needs further supports

	// if (alpha.dat[0] == unity.dat[0] && alpha.dat[1] == unity.dat[1] && beta.dat[0] == complexZero.dat[0] && beta.dat[1] == complexZero.dat[1]) 
	

		// printf("a rows = %d \n", A->size1);
		// printf("a cols = %d \n", A->size2);
		// printf("b rows = %d \n", B->size1);
		// printf("b cols = %d \n", B->size2);
		// KernelEnter("GEMM");
		DASH_GEMM(A_real, A_imag, B_real, B_imag, C_real, C_imag, A->size1, A->size2, B->size2);
		// KernelExit("GEMM");
	

	for (int i = 0; i < C->size1; i++) {
		for (int j = 0; j < C->size2; j++) {
			gsl_matrix_complex_set(C, i, j, gsl_complex_rect(C_real[i * C->size2 + j], C_imag[i * C->size2 + j]));
		}
	}
	
}
//================ Matt's LU-factorization approach ================================================

void LU_factorization_projection(gsl_matrix_complex *Z_temp_proj, int numRx, gsl_matrix_complex *S_temp_delay,gsl_matrix_complex *S_temp_delay_cpy,
                                 int Ntaps_projection, int modulo_N) {
	gsl_complex temp_complex = gsl_complex_rect(1., 0.);

	// calculate the auto-covariance matrix
	gsl_complex unity = gsl_complex_rect(1., 0.);        // the complex number 1
	gsl_complex complexZero = gsl_complex_rect(0., 0.);  // the complex number 0
	gsl_matrix_complex *auto_corr = NULL;
	auto_corr = gsl_matrix_complex_alloc(Ntaps_projection, Ntaps_projection);

    KernelEnter("GEMM");
	printf("S_temp_delay = %p\n", S_temp_delay);
	printf("S_temp_delay_cpy = %p\n", S_temp_delay_cpy);
	TransToDashGemm(CblasNoTrans, CblasConjTrans, unity, S_temp_delay, S_temp_delay_cpy, complexZero, S_temp_delay_cpy);
	gsl_blas_zgemm(CblasNoTrans, CblasConjTrans, unity, S_temp_delay, S_temp_delay, complexZero, auto_corr);
    KernelExit("GEMM");

	// gsl_permutation *p = gsl_permutation_alloc(Ntaps_projection);
	// int s;

		
	// gsl_linalg_complex_LU_decomp(auto_corr, p, &s);

	// // Compute the  inverse of the LU decomposition
	// gsl_matrix_complex *invAutoCorr = NULL;
	// invAutoCorr = gsl_matrix_complex_alloc(Ntaps_projection, Ntaps_projection);
	// gsl_linalg_complex_LU_invert(auto_corr, p, invAutoCorr);


	// gsl_permutation_free(p);

	// // Do the orthogonal projection
	// gsl_matrix_complex *temp_data = NULL;
	// temp_data = gsl_matrix_complex_alloc(Ntaps_projection, modulo_N);
	// gsl_matrix_complex *projection = NULL;
	// projection = gsl_matrix_complex_alloc(modulo_N, modulo_N);


	// KernelEnter("GEMM");
	// printf("S_temp_delay = %p\n", S_temp_delay);
	// printf("S_temp_delay_cpy = %p\n", invAutoCorr);
	// TransToDashGemm(CblasNoTrans, CblasNoTrans, unity, invAutoCorr, S_temp_delay, complexZero, temp_data);
	// // gsl_blas_zgemm(CblasNoTrans, CblasConjTrans, unity, invAutoCorr, S_temp_delay, complexZero, temp_data);
    // KernelExit("GEMM");

	



	// TransToDashGemm(CblasConjTrans, CblasNoTrans, unity, S_temp_delay, temp_data, complexZero, projection);


	// TransToDashGemm(CblasNoTrans, CblasNoTrans, unity, Z_temp_proj, projection, complexZero, temp_data);


	// // orthogonally project by subtracting the projected data from the original data


	// for (int i = 0; i < numRx; i++)  // rows of the received data matrix
	// {
	// 	for (int j = 0; j < modulo_N; j++) {
	// 		// do the subtraction
	// 		temp_complex =
	// 		    gsl_complex_sub(gsl_matrix_complex_get(Z_temp_proj, i, j), gsl_matrix_complex_get(temp_data, i, j));
	// 		// replace the original received data with the data projected onto subspace orthogonal to S
	// 		gsl_matrix_complex_set(Z_temp_proj, i, j, temp_complex);
	// 	}
	// }

	// Now we can replace the relevant section of the received data matrix with this newly projected version
}
//================ End LU-factorization approach ================================================

//================ Pulse-Doppler approach ================================================
void xcorr(double *x, double *y, size_t n_samp, double *corr) {

	size_t len = 2 * n_samp - 1;

	double *c = malloc(2 * len * sizeof(double));
	double *d = malloc(2 * len * sizeof(double));


	size_t x_count = 0;
	size_t y_count = 0;

	// The logic inside this loop seems complex and potentially error-prone
	// relating indices i, x_count, y_count to n_samp and len.
	// Original logic preserved, but careful review might be needed if issues persist.
	for (size_t i = 0; i < 2 * len; i += 2) {
		// Check bounds for x access: x_count should be < 2 * n_samp
		if (i / 2 >= n_samp -1 && x_count < 2 * n_samp) { // Original check was i/2 > n_samp-1
			c[i] = x[x_count];
			c[i + 1] = x[x_count + 1];
			x_count += 2;
		} else {
			c[i] = 0;
			c[i + 1] = 0;
		}

		// Check bounds for y access: y_count should be < 2 * n_samp
		if (i/2 < n_samp && y_count < 2 * n_samp) { // Original check was i > n_samp (maybe meant i/2 ?)
			d[i] = y[y_count];
			d[i + 1] = y[y_count + 1];
			y_count += 2;
		} else {
			d[i] = 0;
			d[i + 1] = 0;
		}
	}


	double *X1 = malloc(2 * len * sizeof(double));
	double *X2 = malloc(2 * len * sizeof(double));
	double *corr_freq = malloc(2 * len * sizeof(double));
	// Add NULL checks for malloc


    //KERN_ENTER(make_label("FFT[1D][%d][complex][float64][forward]",len));
	KernelEnter("FFT");
	gsl_fft(c, X1, len);
	KernelExit("FFT");
    //KERN_EXIT(make_label("FFT[1D][%d][complex][float64][forward]",len));
    //KERN_ENTER(make_label("FFT[1D][%d][complex][float64][forward]",len));
	KernelEnter("FFT");
	gsl_fft(d, X2, len);
	KernelExit("FFT");
    //KERN_EXIT(make_label("FFT[1D][%d][complex][float64][forward]",len));

    //KERN_ENTER(make_label("ZIP[multiply][complex][float64][%d]",len));
	for (size_t i = 0; i < 2 * len; i += 2) {
		corr_freq[i] = (X1[i] * X2[i]) + (X1[i + 1] * X2[i + 1]);
		corr_freq[i + 1] = (X1[i + 1] * X2[i]) - (X1[i] * X2[i + 1]);
	}
    //KERN_EXIT(make_label("ZIP[multiply][complex][float64][%d]",len));

    //KERN_ENTER(make_label("FFT[1D][%d][complex][float64][backward]",len));
	KernelEnter("FFT");
	gsl_ifft(corr_freq, corr, len);
	KernelExit("FFT");
    //KERN_EXIT(make_label("FFT[1D][%d][complex][float64][backward]",len));

	// Free intermediate arrays
	free(c);
	free(d);
	free(X1);
	free(X2);
	free(corr_freq);
}

//================ End radar correlator approach ================================================