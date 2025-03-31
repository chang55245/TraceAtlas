#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "CyclicPrefix.h"
#include "Preamble_ST_LG.h"
#include "assert.h"
#include "baseband_lib.h"
#include "ch_Est_Equa.h"
#include "channel_Eq.h"
#include "channel_Eq_initialize.h"
#include "channel_Eq_terminate.h"
#include "common.h"
#include "datatypeconv.h"
#include "decode.h"
#include "detection.h"
#include "dma.h"
#include "equalizer.h"
#include "fft_hs.h"
#include "fft_hwa.h"
#include "interleaver_deintleaver.h"
#include "pilot.h"
#include "qpsk_Mod_Demod.h"
#include "rfInf.h"
#include "rf_interface.h"
#include "rt_nonfinite.h"
#include "scrambler_descrambler.h"
#include "viterbi.h"
#include "dash.h"

void get_tx_fft_data();
void get_rx_viterbi_data();
void get_rx_demod_data();


#define SEC2NANOSEC 1000000000

//############################
//## FFT
//############################
extern TYPE *udmabuf1_base_addr;
extern int dma1_control_fd;
extern unsigned int *dma1_control_base_addr;
extern int fd_udmabuf1;
extern int fft1_control_fd;
extern unsigned int *fft1_control_base_addr;
extern TYPE *udmabuf2_base_addr;
extern int dma2_control_fd;
extern unsigned int *dma2_control_base_addr;
extern int fd_udmabuf2;
extern int fft2_control_fd;
extern unsigned int *fft2_control_base_addr;

TYPE *udmabuf1_base_addr;
int dma1_control_fd;
unsigned int *dma1_control_base_addr;
int fd_udmabuf1;
int fft1_control_fd;
unsigned int *fft1_control_base_addr;
TYPE *udmabuf2_base_addr;
int dma2_control_fd;
unsigned int *dma2_control_base_addr;
int fd_udmabuf2;
int fft2_control_fd;
unsigned int *fft2_control_base_addr;

static char attr[1024];
int fft_id;

extern sem_t mutex;
sem_t mutex;

signed char dec_in[OUTPUT_LEN];
unsigned char dec_out[USR_DAT_LEN];
signed char dec_pun_out[OUTPUT_LEN];
signed char deintl_out[OUTPUT_LEN];
double out_real[OUTPUT_LEN];
double out_img[OUTPUT_LEN];
signed char outbit[OUTPUT_LEN];
unsigned char descram[USR_DAT_LEN * SYM_NUM + 1];
int hw_fft_busy;

unsigned int *base_addr;

int main() {
	int fft_a53_count = 0;
	int fft_acc_count = 0;

	float FFT_RUN_ON_CPU_PROBABILITY = 0.5;


	FFT_RUN_ON_CPU_PROBABILITY = 0.5;

	int j;

	int decoderId;
	
	int frameCount = 0;
	FILE *fp;
	struct timespec start1, end1;
	float exec_time;


	init_viterbiDecoder();
	decoderId = get_viterbiDecoder();
	set_viterbiDecoder(decoderId);

	// init_equalizer();

	int frame_count = 0;

	comp_t *input_fd = (comp_t *)malloc(FFT_N * 2 * sizeof(double));
	comp_t *output_fd = (comp_t *)malloc(FFT_N * 2 * sizeof(double));
	
	comp_t *pilot_rm = (comp_t *)malloc(INPUT_LEN * 2 * sizeof(double));
	double *offt;
	double *pilotdata_rx = (double *)malloc(PILOT_LEN * 2 * sizeof(double));
	double *out_fft_c2f = (double *)malloc(INPUT_LEN * 2 * sizeof(double)*2);

		//............... Frame Detection ........................
		if (frameDetection() == 0) frameDetection();

		// clean RX buffer
		for (j = 0; j < SYM_NUM * USR_DAT_LEN; j++) descram[j] = 0;

		for (j = 0; j < SYM_NUM; j++) {
			NonKernelSplit();
#ifdef ACC_RX_FFT
			get_tx_fft_data();
#endif

#ifdef ACC_RX_DEMOD
			get_rx_demod_data();
#endif

#ifdef ACC_RX_DECODER
			get_rx_viterbi_data();
#endif
			
			// srand(clock());
			srand(frame_count + 1);

			payloadExt(input_fd);

			//###############################################################
			//## FFT
			//###############################################################
			int random_wait_time_in_us;  // = 15;
			float random_wait;
			float random_wait1;
			float random_wait2;
			float run_on_cpu;

			fft_id = 1;

			hw_fft_busy = 1;


			 double *ifft_input = (double*)malloc(sizeof(double)*FFT_N*2);


			 for (int k = 0; k < FFT_N; k++){
                ifft_input[k*2] = (double)input_fd[k].real;
                ifft_input[k*2+1] = (double)input_fd[k].imag;
              }
			 double *ifft_output = (double *)malloc(FFT_N * 2 * sizeof(double));
			bool forwardTrans = false;
			size_t size = FFT_N;
			KernelEnter("FFT");
              // printf("DASH_FFT %p, %d\n",ifft_input, i);
			DASH_FFT(ifft_input, ifft_output, size, forwardTrans);
			KernelExit("FFT");

			for (int k = 0; k < FFT_N; k++){
				output_fd[k].real = ifft_output[k*2];
				output_fd[k].imag = ifft_output[k*2+1];
			}
			free(ifft_input);
			free(ifft_output);

			pilotRemove(FFT_N, output_fd, pilot_rm);


			for (int i = 0; i < INPUT_LEN; i++) {
				out_fft_c2f[2 * i] = pilot_rm[i].real;
				out_fft_c2f[2 * i + 1] = pilot_rm[i].imag;
			}

			//###############################################################
			//## QPSK Demodulation
			//###############################################################


			comp_t *ibuf = (comp_t *)pilot_rm;
			signed char *out = (signed char *)outbit;
			DeMOD_QPSK(INPUT_LEN, ibuf, out);


			deinterleaver(outbit, OUTPUT_LEN, deintl_out);



			formatConversion(PUNC_RATE_1_2, deintl_out, dec_in);


			viterbi_decoding(decoderId, dec_in, dec_out);


			descrambler(USR_DAT_LEN, dec_out, &descram[j * USR_DAT_LEN]);

			NonKernelSplit();
		}

		messagedecoder((unsigned char *)descram);

		frame_count++;
		

	printf("Done!\n");
    
}