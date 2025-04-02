#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "dash.h"

int main(void)
{
    clock_t start = clock();
    size_t len, x_count, y_count, i, j, k;
    size_t n_samples; 
    size_t dft_size; 

    double lag;
    double T;
    double B;
    double sampling_rate;
    double max_corr;
    double index;

    double *c, *d, *X1, *X2, *corr_freq;
    double *time, *received, *corr, *gen_wave;


    FILE *fp;

    int row, column, row2, column2;

    n_samples = 128;
    //dft_size = 2 * n_samples - 1;
    dft_size = 2 * n_samples;
    len = 2 * n_samples;

    T = 0.000512;
    B = 500000;
    sampling_rate = 1000;
    max_corr = 0;
    index = 0;

    time = malloc(n_samples * sizeof(double));
    received = malloc(len * sizeof(double));
    corr = malloc(2 * len * sizeof(double));
    gen_wave = malloc(len * sizeof(double));

    fp = fopen("./input/time_input.txt","r");
    if (fp == NULL) { printf("Unable to open time_input.txt!\n"); }
    for(i = 0; i < n_samples; i++) {
        fscanf(fp, "%lf", &time[i]);
    }
    fclose(fp);
    fp = NULL;

    for (i = 0; i < len; i += 2) {
        gen_wave[i] = sin(M_PI * B / T * pow(time[i / 2], 2));
        gen_wave[i + 1] = cos(M_PI * B / T * pow(time[i / 2], 2));
    }

    fp = fopen("./input/received_input.txt","r");
    if (fp == NULL) { printf("Unable to open received_input.txt!\n"); }
    for(i = 0; i < len; i++) {
        fscanf(fp,"%lf", &received[i]);
    }
    fclose(fp);
    fp = NULL;

    c = malloc(2 * len * sizeof(double));
    d = malloc(2 * len * sizeof(double));

    x_count = 0;
    y_count = 0;
    for (i = 0; i < 2 * len; i += 2) {
        if (i/2 > n_samples - 1) {
            c[i] = gen_wave[x_count];
            c[i + 1] = gen_wave[x_count + 1];
            x_count += 2;
        } else {
            c[i] = 0;
            c[i + 1] = 0;
        }
        if (i > n_samples) {
            d[i] = 0;
            d[i + 1] = 0;
        } else {
            d[i] = received[y_count];
            d[i + 1] = received[y_count + 1];
            y_count += 2;
        }
    }

    X1 = malloc(2 * len * sizeof(double));
    X2 = malloc(2 * len * sizeof(double));
    corr_freq = malloc(2 * len * sizeof(double));

    clock_t t0 = clock();

    printf("time0: %d \n", (int)(t0-start));
   
    KernelEnter("FFT");
    DASH_FFT(c, X1, len, true);
    KernelExit("FFT");

    clock_t fft0 = clock();

    printf("fft0: %d \n", (int)(fft0-t0));

    KernelEnter("FFT");
    DASH_FFT(d, X2, len, true);
    KernelExit("FFT");

    clock_t fft1 = clock();

    // printf("fft1: %d \n", (int)(fft1-fft0));

    for (i = 0; i < 2 * len; i += 2) {
        corr_freq[i] = (X1[i] * X2[i]) + (X1[i + 1] * X2[i + 1]);
        corr_freq[i + 1] = (X1[i + 1] * X2[i]) - (X1[i] * X2[i + 1]);
    }

    clock_t t1 = clock();

    printf("t1: %d \n", (int)(t1-fft1));

    KernelEnter("FFT");
    DASH_FFT(corr_freq, corr, len, false);
    KernelExit("FFT");

    clock_t fft2 = clock();

    printf("fft2: %d \n", (int)(fft2-t1));


    for (i = 0; i < 2 * len; i += 2)
    {
        if (corr[i] > max_corr) {
            max_corr = corr[i];
            index = i / 2;
        }
    }

    lag = (n_samples - index) / sampling_rate;

    clock_t t3 = clock();

    printf("t3: %d \n", (int)(t3-fft2));

    printf("Radar correlator complete; lag Value is: %lf\n", lag);

    free(time);    
    free(received);
    free(corr);
    free(gen_wave);
    free(c);
    free(d);
    free(X1);
    free(X2);
    free(corr_freq);

    printf("[Radar Correlator] Execution is complete...\n");
}
