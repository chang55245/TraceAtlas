#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>                       
// #define EXCLUDE_DUMMY
#include<unistd.h>

// #undef EXCLUDE_DUMMY
#include "dash.h" 
#include <sys/time.h>                                       
#define PROGPATH "/home/liangliangdev/cedr/zynq_hardware_emulator/applications/APIApps/pulse_doppler/input/"               
#define PDPULSE PROGPATH "input_pd_pulse.txt"
#define PDPS PROGPATH "input_pd_ps.txt"   
#define OUTPUT PROGPATH "output_pd_f.txt" 
                                              
/* Function Declarations */               
void xcorr(double *, double *, size_t, double *);
void swap(double *, double *);  
void fftshift(double *, double);
                     
/* Function Definitions */                
void xcorr(double *x, double *y, size_t n_samp, double *corr) {

  // clock_t beforefft1 = clock();
  

  size_t len;    
  len = 2 * n_samp;  
  double *c = malloc(2 * len * sizeof(double));
  double *d = malloc(2 * len * sizeof(double));
                                          
  size_t x_count = 0;
  size_t y_count = 0;

  double *z = malloc(2 * (n_samp) * sizeof(double));    
  for (size_t i = 0; i < 2 * (n_samp); i += 2) { 
    z[i] = 0;    
    z[i + 1] = 0;
  }                  
  for (size_t i = 0; i < 2 * (n_samp - 1); i += 2) {                  
    c[i] = 0;    
    c[i + 1] = 0;
  }                                       
  memcpy(c + 2 * (n_samp - 1), x, 2 * n_samp * sizeof(double));   
  c[2 * len - 2] = 0;
  c[2 * len - 1] = 0;
  memcpy(d, y, 2 * n_samp * sizeof(double));   
  memcpy(d + 2 * n_samp, z, 2 * (n_samp) * sizeof(double)); 
  double *X1 = malloc(2 * len * sizeof(double));        
  double *X2 = malloc(2 * len * sizeof(double));        
  double *corr_freq = malloc(2 * len * sizeof(double));


  // clock_t beforefft2 = clock();
  // printf("corr: %d \n", (int)(beforefft2-beforefft1));

 
  // gsl_fft(c, X1, len);
  KernelEnter("FFT");
  printf("22");   
  DASH_FFT(c, X1, len, true /* is_forward_transform? */);
  KernelExit("FFT");
  
  // gsl_fft(d, X2, len);
  KernelEnter("FFT");
  printf("33");
  DASH_FFT(d, X2, len, true /* is_forward_transform? */);
  KernelExit("FFT");

  // free(c);
  // free(d);
  // free(z);


//  clock_t start = clock();

  for (size_t i = 0; i < 2 * len; i += 2) {
    corr_freq[i] = (X1[i] * X2[i]) + (X1[i + 1] * X2[i + 1]);
    corr_freq[i + 1] = (X1[i + 1] * X2[i]) - (X1[i] * X2[i + 1]);
  }

  // usleep(1000);

  // clock_t end = clock();
  // printf("corr: %d \n", (int)(end-start));


  // free(X1);
  // free(X2);


  struct timespec  fft_start_t, fft_end_t;
  
  
  KernelEnter("FFT");
  // clock_t start2 = clock();
  // clock_gettime(CLOCK_MONOTONIC_RAW, &fft_start_t);
  DASH_FFT(corr_freq, corr, len, false /* is_forward_transform? */);

  // clock_gettime(CLOCK_MONOTONIC_RAW, &fft_end_t);
  
  // double time_taken = (fft_end_t.tv_sec - fft_start_t.tv_sec) + (fft_end_t.tv_nsec - fft_start_t.tv_nsec) / 1000000000.0;
  // printf("FFT: %f \n", time_taken);
  // clock_t end2 = clock();
  // printf("FFT: %d \n", (int)(end2-start2));
  KernelExit("FFT");


  

  // KernelEnter("FFT");
  // DASH_FFT(corr, corr, len, false /* is_forward_transform? */);
  // KernelExit("FFT");

}

void swap(double *v1, double *v2) {
  double tmp = *v1;
  *v1 = *v2;
  *v2 = tmp;
}

void fftshift(double *data, double count) {
  int k = 0;
  int c = (double)floor((float)count / 2);
  // For odd and for even numbers of element use different algorithm
  if ((int)count % 2 == 0) {
    for (k = 0; k < c; k++)
      swap(&data[k], &data[k + c]);
  } else {
    double tmp = data[0];
    for (k = 0; k < c; k++) {
      data[k] = data[c + k + 1];
      data[c + k + 1] = data[k + 1];
    }
    data[c] = tmp;
  }
}


int main(int argc, char *argv[]) {


  struct timespec  start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  // struct timeval start, end;
  // gettimeofday(&start, NULL);

  // double *test_i = malloc(2 * 2 * sizeof(double));
  // double *test_o = malloc(2 * 2 * sizeof(double));
  // memset(test_i, 1, 2 * 2 * sizeof(double));
  // KernelEnter("FFT");
  // DASH_FFT(test_i, test_o, 2, true);
  // KernelExit("FFT");

  // int test_dep = test_o[0];

  // clock_t init1 = clock();

  // size_t m = 256 ;    // number of pulses 256
  // size_t n_samples = 128; // length of single pulse 128
  size_t m = 4;
  size_t n_samples = 2;
  
  double PRI = 1.27e-4;
  int i, j, k, n, x, y, z, o;

  double *mf = malloc((2 * n_samples) * m * 2 *
                      sizeof(double)); // build a 2D array for the output of the
                                       // matched filter
  double *p =
      malloc(2 * n_samples * sizeof(double)); // array for pulse with noise
  double *pulse =
      malloc(2 * n_samples * sizeof(double)); // array for the original pulse
  double *corr =
      malloc(2 * (2 * n_samples) *
             sizeof(double)); // array for the output of matched filter

  // Read the original pulse
  FILE *fp;
  fp = fopen(PDPULSE, "r");

  if(fp == NULL) {
    perror("fopen");
    printf("Error opening file %s\n", PDPULSE);
    exit(1);
  }
  for (i = 0; i < 2 * n_samples; i++) {
    fscanf(fp, "%lf", &pulse[i]);
    // printf("%lf,", pulse[i]);
  }
  fclose(fp);

  printf("11");

  // Run the input samples through the matched filter
  fp = fopen(PDPS, "r"); // read the multiple pulses with noise and delay
  if(fp == NULL) {
    perror("fopen");
    printf("Error opening file %s\n", PDPS);
    exit(1);
  }
  // clock_t init2 = clock();
  // printf("init: %d \n", (int)(init2-init1));
  for (k = 0; k <m; k++) 
  {

    NonKernelSplit();
    for (j = 0; j < 2 * n_samples; j++) {
      fscanf(fp, "%lf", &p[j]);
      // printf("%f,", p[j]);
    }
    // printf("\n");

    /* matched filter */

    xcorr(p, pulse, n_samples, corr);  



    // clock_t afterxcorr1 = clock();
   
   
    /* put the output into a new 2D array */
    for (n = 0; n < 2 * (2 * n_samples); n += 2) {
      mf[n / 2 + (2 * k) * (2 * n_samples)] = corr[n];
      mf[n / 2 + (2 * k + 1) * (2 * n_samples)] = corr[n + 1];

    }

    // clock_t afterxcorr2 = clock();
    // printf("afterxcorr: %d \n", (int)(afterxcorr2-afterxcorr1));
      printf("33");
    NonKernelSplit();
  }
  

  // fclose(fp);
  // free(p);
  // free(pulse);
  // free(corr);

  double *q = malloc(2 * m * sizeof(double));
  double *r = malloc(m * sizeof(double));
  double *l = malloc(2 * m * sizeof(double));
  double *f = malloc(m * (2 * n_samples) * sizeof(double));
  double max = 0, a, b;

  /* FFT */
  


  // #pragma clang loop unroll_count(2000)
  for (x = 0; x < 2 * n_samples; x++) {
       NonKernelSplit();

    // clock_t before2fft1 = clock();
    for (o = 0; o < 2 * m; o++) {
      l[o] = mf[x + o * (2 * n_samples)];
    }

    // clock_t before2fft2 = clock();
    // printf("before2fft: %d \n", (int)(before2fft2-before2fft1));
    
    // gsl_fft(l,q,m);
    KernelEnter("FFT");
    DASH_FFT(l, q, m, true /* is_forward_transform? */);
    KernelExit("FFT");


    // clock_t after2fft1 = clock();

    for (y = 0; y < 2 * m; y += 2) {
      r[y / 2] = sqrt(
          q[y] * q[y] +
          q[y + 1] * q[y + 1]); // calculate the absolute value of the output
    }
    fftshift(r, m);

    for (z = 0; z < m; z++) {
      f[x + z * (2 * n_samples)] = r[z]; // put the elements of output into corresponding location of the 2D array
      if (r[z] > max) {
        max = r[z];
        a = z + 1;
        b = x + 1;
      }
    }

    // clock_t after2fft2 = clock();
    // printf("after2fft: %d \n", (int)(after2fft2-after2fft1));
    NonKernelSplit();
  }
  


  // free(mf);
  // free(q);
  // free(r);
  // free(l);

  double rg, dp;
  rg = (b - n_samples) / (n_samples - 1) * PRI;
  dp = (a - (m + 1) / 2) / (m - 1) / PRI;

  fp = fopen("./output/pulse_doppler_output.txt", "w");
  if (fp != NULL) {
    fprintf(fp, "Doppler shift = %lf, time delay = %lf\n", dp, rg);
    fclose(fp);
  }


  // KernelEnter("FFT");
  // DASH_FFT(&dp, test_o, 1, true);
  // KernelExit("FFT");

  // gettimeofday(&end, NULL);

 //  double delta = ((end.tv_sec  - start.tv_sec) * 1000000u + 
    //      end.tv_usec - start.tv_usec) / 1.e6;

  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
  


  printf("[Pulse Doppler] Execution is complete... %f\n", time_taken);

}
