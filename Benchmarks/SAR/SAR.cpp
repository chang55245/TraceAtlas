#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


#include "dash.h"

#define PROGPATH "/home/lchang21/cedr-josh/zynq_hardware_emulator/applications/APIApps/SAR/input/"
#define RAWDATA PROGPATH "rawdata_rda.txt"
#define OUTPUT "SAR_RDA-output.txt"

// Define KERN_ENTER and KERN_EXIT as NOPs
#define KERN_ENTER(KERN_STR)
#define KERN_EXIT(KERN_STR)

void swap(double *, double *);
void fftshift(double *, double);

void swap(double *v1, double *v2) {
  double tmp = *v1;
  *v1 = *v2;
  *v2 = tmp;
}

void fftshift(double *data, double count) {
  int k = 0;
  int c = (double)floor(count / 2);
  // For odd and for even numbers of element use different algorithm
  if ((int)count % 2 == 0) {
    for (k = 0; k < 2 * c; k += 2) {
      swap(&data[k], &data[k + 2 * c]);
      swap(&data[k + 1], &data[k + 1 + 2 * c]);
    }
  } else {
    double tmp1 = data[0];
    double tmp2 = data[1];
    for (k = 0; k < 2 * c; k += 2) {
      data[k] = data[2 * c + k + 2];
      data[k + 1] = data[2 * c + k + 3];
      data[2 * c + k + 2] = data[k + 2];
      data[2 * c + k + 3] = data[k + 3];
    }
    data[2 * c] = tmp1;
    data[2 * c] = tmp2;
  }
}

int main(void) {
  struct timespec  start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);
  double c = 3e8;

  int i, j;

  int Nslow;
  int Nfast;
  double v;
  double Xmin;
  double Xmax;
  double Yc;
  double Y0;
  double Tr;
  double Kr;
  double h;
  double lambda;

  double R0;
  double Ka;
  double *s0;

  FILE *fp;

  double *ta;

  double Rmin, Rmax;
  double *tr;

  double *g;
  double *src;
  double *fft_arr;
  double *temp;
  double *temp2;
  double *temp3;
  double *g2;

  double *S1;
  double *fft_arr_2;
  double *temp4;

  // Azimuth Compression
  double *H;
  double *sac;
  double *fft_arr_4;
  double *temp8;
  double *temp9;
  
  // printf("[SAR] Starting execution of the non-kernel thread\n");

  Nslow = 256;
  Nfast = 512;
  v = 150;
  Xmin = 0;
  Xmax = 50;
  Yc = 10000;
  Y0 = 500;
  Tr = 2.5e-6;
  Kr = 2e13;
  h = 5000;
  lambda = 0.0566;

  R0 = sqrt(Yc * Yc + h * h);
  Ka = 2 * v * v / lambda / R0;
  s0 = (double*) malloc(2 * Nslow * Nfast * sizeof(double));
  
  fp = fopen(RAWDATA, "r");
  for (i = 0; i < 2 * Nslow * Nfast; i++) {
    fscanf(fp, "%lf", &s0[i]);
  }
  fclose(fp);

  ta = (double*) malloc(Nslow * sizeof(double));
  ta[0] = 0;
  for (i = 1; i < Nslow; i++) {
    ta[i] = ta[i - 1] + (Xmax - Xmin) / v / (Nslow - 1);
  }

  Rmin = sqrt((Yc - Y0) * (Yc - Y0) + h * h);
  Rmax = sqrt((Yc + Y0) * (Yc + Y0) + h * h);
  tr = (double*) malloc(Nfast * sizeof(double));
  tr[0] = 0;
  for (i = 1; i < Nfast; i++) {
    tr[i] = tr[i - 1] + (2 * Rmax / c + Tr - 2 * Rmin / c) / (Nfast - 1);
  }
  
  g = (double*) malloc(2 * Nfast * sizeof(double));
  for (i = 0; i < 2 * Nfast; i += 2) {
    if (tr[i / 2] > -Tr / 2 && tr[i / 2] < Tr / 2) {
      g[i] = cos(M_PI * Kr * tr[i / 2] * tr[i / 2]);
      g[i + 1] = -sin(M_PI * Kr * tr[i / 2] * tr[i / 2]);
    } else {
      g[i] = 0;
      g[i + 1] = 0;
    }
  }
  size_t fast = Nfast;
  size_t slow = Nslow;
  bool forwardTrans = true;
  
  src = (double*) malloc(2 * Nfast * Nslow * sizeof(double));
  fft_arr = (double*) malloc(2 * Nfast * sizeof(double));
  temp = (double*) malloc(2 * Nfast * sizeof(double));
  temp2 = (double*) malloc(2 * Nfast * sizeof(double));
  temp3 = (double*) malloc(2 * Nfast * sizeof(double));
  g2 = (double*) malloc(2 * Nfast * sizeof(double));
  
  // printf("[SAR] Enqueuing my kernel, DASH_FFT\n");  
  KernelEnter("FFT");
  DASH_FFT(g, g2, fast, forwardTrans);
  KernelExit("FFT");
  printf("111111\n");
  
  for (i = 0; i < Nslow; i++) {
    NonKernelSplit();
    for (j = 0; j < 2 * Nfast; j++) {
      fft_arr[j] = s0[j + i * 2 * Nfast];
    }
    KernelEnter("FFT");
    DASH_FFT(fft_arr, temp, fast, true);
    KernelExit("FFT");
    
    fftshift(temp, Nfast);
    // KERN_ENTER(make_label("ZIP[multiply][%d][float64][complex]", Nfast));
    for (j = 0; j < 2 * Nfast; j += 2) {
      temp2[j] = temp[j] * g2[j] - temp[j + 1] * g2[j + 1];
      temp2[j + 1] = temp[j + 1] * g2[j] + temp[j] * g2[j + 1];
	  //printf("[temp2] %d: %lf\n", j, temp2[j]);
    }
    // KERN_EXIT(make_label("ZIP[multiply][%d][float64][complex]", Nfast));
    KernelEnter("FFT");
    DASH_FFT(temp2, temp3, fast, false);
    KernelExit("FFT");
    for (j = 0; j < 2 * Nfast; j += 2) {
      src[j * Nslow + 2 * i] = temp3[j];
      src[j * Nslow + 2 * i + 1] = temp3[j + 1];
    }
    NonKernelSplit();
  }


  printf("222222\n");

  int test = 0;
  // if(test)
  {
  // Azimuth FFT
  S1 = (double*) malloc(2 * Nfast * Nslow * sizeof(double));
  fft_arr_2 = (double*) malloc(2 * Nslow * sizeof(double));
  temp4 = (double*) malloc(2 * Nslow * sizeof(double));
  for (i = 0; i < Nfast; i++) {
    NonKernelSplit();
    for (j = 0; j < 2 * Nslow; j += 2) {
      fft_arr_2[j] = src[j + i * 2 * Nslow];
      fft_arr_2[j + 1] = src[j + 1 + i * 2 * Nslow];
    }
    KernelEnter("FFT");
	  DASH_FFT(fft_arr_2, temp4, slow, true);
    KernelExit("FFT");
    fftshift(temp4, Nslow);
    for (j = 0; j < 2 * Nslow; j += 2) {
      S1[j + i * 2 * Nslow] = temp4[j];
      S1[j + 1 + i * 2 * Nslow] = temp4[j + 1];	  
    }
    NonKernelSplit();
  }
  // }
  // Azimuth Compression
  H = (double*) malloc(2 * Nslow * sizeof(double));
  for (i = 0; i < 2 * Nslow; i += 2) {
    if (ta[i / 2] > -Tr / 2 * (Xmax - Xmin) / v / (2 * Rmax / c + Tr - 2 * Rmin / c) &&
        ta[i / 2] < Tr / 2 * (Xmax - Xmin) / v / (2 * Rmax / c + Tr - 2 * Rmin / c)) {
      H[i] = cos(M_PI * Ka * ta[i / 2] * ta[i / 2]);
      H[i + 1] = sin(M_PI * Ka * ta[i / 2] * ta[i / 2]);
    } else {
      H[i] = 0;
      H[i + 1] = 0;
    }
  }
  sac = (double*) malloc(Nslow * Nfast * sizeof(double));
  fft_arr_4 = (double*) malloc(2 * Nslow * sizeof(double));
  temp8 = (double*) malloc(2 * Nslow * sizeof(double));
  temp9 = (double*) malloc(2 * Nslow * sizeof(double));
  for (i = 0; i < Nfast; i++) {
    NonKernelSplit();
    for (j = 0; j < 2 * Nslow; j++) {
      temp8[j] = S1[j + i * 2 * Nslow];
	  //if(S1[j + i * 2 * Nslow] > 0.000001)
		//printf("[S1] %d: %lf\n", (j + i * 2 * Nslow), S1[j + i * 2 * Nslow]);
    }
    //KERN_ENTER(make_label("ZIP[multiply][%d][float64][complex]", Nslow));
    for (j = 0; j < 2 * Nslow; j += 2) {
      fft_arr_4[j] = temp8[j] * H[j] - temp8[j + 1] * H[j + 1];
      fft_arr_4[j + 1] = temp8[j + 1] * H[j] + temp8[j] * H[j + 1];
	  //if(fft_arr_4[j] > 0.000001)
		//printf("[fft_arr_4] %d: %lf\n", j, fft_arr_4[j]);
    }
    //KERN_EXIT(make_label("ZIP[multiply][%d][float64][complex]", Nslow));
    KernelEnter("FFT");
    DASH_FFT(fft_arr_4, temp9, slow, false);
    KernelExit("FFT");
    fftshift(temp9, Nslow);
    for (j = 0; j < Nslow; j++) {
      sac[i + j * Nfast] = sqrt(temp9[2 * j] * temp9[2 * j] + temp9[2 * j + 1] * temp9[2 * j + 1]);
    }
    NonKernelSplit();
	
  }
  
  fp = fopen("/home/lchang21/cedr-josh/zynq_hardware_emulator/applications/APIApps/SAR/output/SAR_output.txt", "w");
  if (fp != NULL) {
    for (i = 0; i < Nslow; i++) {
      for (j = 0; j < Nfast; j++) {
        fprintf(fp, "%lf ", sac[j + i * Nfast]);
      }
      fprintf(fp, "\n");
      fflush(fp);
    }
    fclose(fp);
  }
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
  

  printf("[SAR] Execution is complete...,%f \n", time_taken);



}
  return 0;
}
