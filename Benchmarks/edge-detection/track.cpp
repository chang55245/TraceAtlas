#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
//#include <math.h>
#include <cstdio>
#include <cstddef>
#include <fstream>
//#include <math.h>
#include <iomanip> 
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>


#include "conv.hpp"
//#include "dash.h"

#define SIGMA 0.75
#define PI 3.141593

// Header files for image read/write
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

struct Line{
    int x1, y1, x2, y2;

    Line(int x1, int y1, int x2, int y2): x1(x1), y1(y1), x2(x2), y2(y2){};
};

struct Point{
    int x, y;

    Point(int x, int y): x(x), y(y){};
};

double area(int x1, int y1, int x2, int y2, int x3, int y3){
    return abs((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2);
}

double power(double x, int n)
{
    double pow = 1.0;
    for (int i = 0; i < n; i++) {
        pow = pow * x;
    }
    return pow;
}

int power(int x, int n)
{
    int pow = 1.0;
    for (int i = 0; i < n; i++) {
        pow = pow * x;
    }
    return pow;
}

// Grayscale conversion operation
// Input: 
// img --> 3D image that is going to be grayscaled
// height --> image height
// width --> image width
// depth --> image depth(# of color channels)
// Output:
// return_array --> 2D grayscaled image
void grayscale_conversion(double *img, int height, int width, int depth, double *return_array){
    double r, g, b, gray;
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
                r = (double)img[(i * width + j) * depth + 0];
                g = (double)img[(i * width + j) * depth + 1];
                b = (double)img[(i * width + j) * depth + 2];

                r *= 0.299;
                g *= 0.587;
                b *= 0.144;

                gray = (r + g + b);
                return_array[i * width + j] = gray;
        }
    }
}


// Gaussian blur operation
// Input:
// img --> image that is going to blurred
// kernel_size --> kernel size for filter, filter[kernel_size][kernel_size] is created.
// height --> image height
// width --> image width
// Output: return_array --> blurred_img
void gaussian_blur(double *img, int kernel_size, int height, int width, double *return_array){
    double r, g, p, s, e;
    double sum = 0;
    double flattened_kernel[kernel_size * kernel_size];


    // Computation of gaussian filter
    for(int x = 0; x < kernel_size; x++){
        for(int y = 0; y < kernel_size; y++){
            r = power(x, 2) + power(y, 2);
            s = 2 * power(SIGMA, 2);
            s *= PI;
            p = -(r / s);
            e = exp(p);
            g = e / s;
            flattened_kernel[x*kernel_size+y] = g;
            sum += g;
        }
    }

    for(int i = 0; i < kernel_size; i++){
        for(int j = 0; j < kernel_size; j++){
	        flattened_kernel[i*kernel_size+j] /= sum;
        }
    }
//#ifdef SERIAL
    //conv_serial(img, flattened_kernel, return_array, height, width, kernel_size);
//#elif FFT
    conv_fft(img, flattened_kernel, return_array, height, width, kernel_size);
//#endif
}

void sobel_g_operator(double *img, double *filter, int kernel_size, int height, int width, double *return_array){
//#ifdef SERIAL
    //conv_serial(img, filter, return_array, height, width, kernel_size);
//#elif FFT
    conv_fft(img, filter, return_array, height, width, kernel_size);
//#endif
}

void sobel(double *gx, double *gy, int height, int width, int lower_threshold, int upper_threshold, double *return_array){

    // Computing the resultant sobel frame using generated gx and gy frames. Equation is taken from the original method.
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            return_array[i * width + j] = sqrt(power(gx[i * width + j], 2) + power(gy[i * width + j], 2));
        }
    }

    double *angle = (double *) malloc (sizeof (double) * (height * width));
    // Computing angles to decide which pixels are going to be omitted later in the next step.
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            angle[i * width + j] = atan2(gy[i * width + j], gx[i * width + j]) * 180 / PI;
            if(angle[i * width + j] < 0) angle[i * width + j] += 180;
        }
    }

    // Checking nearby frames according to the associated angle, omitting if necessary.
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            if(i == 0 || j == 0 || i == height - 1 || j == width - 1){
                return_array[i * width + j] = 0;
            }
            else{
                if(angle[i * width + j] == 0){
                    if((return_array[i * width + j] < return_array[i * width + j - 1]) || (return_array[i * width + j] < return_array[i * width + j + 1])) 
                        return_array[i * width + j] = 0;
                }
                if(angle[i * width + j] == 90){
                    if((return_array[i * width + j] < return_array[(i - 1) * width + j]) || (return_array[i * width + j] < return_array[(i + 1) * width + j])) 
                        return_array[i * width + j] = 0;
                }
                if(angle[i * width + j] == 45){
                    if((return_array[i * width + j] < return_array[(i - 1) * width + j + 1]) || (return_array[i * width + j] < return_array[(i + 1) * width + j - 1])) 
                        return_array[i * width + j] = 0;
                }
                if(angle[i * width + j] == 135){
                    if((return_array[i * width + j] < return_array[(i - 1) * width +  j - 1]) || (return_array[i * width + j] < return_array[(i + 1) * width + j + 1])) 
                        return_array[i * width + j] = 0;
                }
            }
                
                
            if(return_array[i * width + j] < lower_threshold)  return_array[i * width + j] = 0;
            else if (return_array[i * width + j] > lower_threshold && return_array[i * width + j] < upper_threshold){
                if((return_array[(i - 1) * width + j] > upper_threshold) || (return_array[(i + 1) * width + j] > upper_threshold) || 
                    (return_array[i * width + j - 1] > upper_threshold) || (return_array[i * width + j + 1] > upper_threshold))continue;
                else return_array[i * width + j] = 0;
            }
        }
    }
}

void save_img(double *img, int height, int width, char* name){
    uint8_t* out_img = (uint8_t *) malloc (sizeof (uint8_t) * (height * width));
    for (int i = 0; i<(height * width); i++){
        out_img[i] = (uint8_t)img[i];
    }
    stbi_write_png(name, width, height, 1, out_img, width);
    free(out_img);
}

int main(int argc, char *argv[]){

    struct timespec  start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    // Change height and width manually
    // for normal image height is 540 and width is 960
    // for 4x downscaled height is 135 and width is 240
    // for 10x downscaled height os 54 and width is 96
    int height = 135;
    int width = 240;
    int depth = 3;
    int size = height * width;

    // Allocating memory for frame holder arrays.
    double *temp = (double *) malloc (sizeof (double) * (size*3));
    double *img = (double *) malloc (sizeof (double) * (size * 3));
    double *grayscale_img = (double *) malloc (sizeof (double) * size);
    double *blurred_img = (double *) malloc (sizeof (double) * size);
    double *blurred_img_before_canny = (double *) malloc (sizeof (double) * size);
    double *gx_canny = (double *) malloc (sizeof (double) * size);
    double *gy_canny = (double *) malloc (sizeof (double) * size);
    double *sobel_img = (double *) malloc (sizeof (double) * size);
    double *lanes = (double *) malloc (sizeof (double) * size);
    
    int idx = 0;
    FILE *fp;
    char *filename = "input_4x_downscaled.txt"; 
    char line[100];
    double num;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error: unable to open input file.\n");
        return 1;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        num = atof(line);
        temp[idx++] = num;
    }
    fclose(fp);
    
    for(int i = 0; i < height * width * 3; i++){
        img[i] = (double) temp[i];
    }
	
    // Convertion to grayscale. [HEIGHT][WIDTH][DEPTH] --> [HEIGHT][DEPTH]
    // Doing in main to only deal with 2 dimensional frame from now on.
    // Flattening 2D frame to 1D array for better handling with methods.
    grayscale_conversion(img, height, width, depth, grayscale_img);

//#ifdef PRINT_IMAGES
    //save_img(grayscale_img, height, width, "grayscale_img.png");
//#endif

    // First gaussian blur operation. Kernel size is chosen as 11. 
    gaussian_blur(grayscale_img, 11, height, width, blurred_img);

//#ifdef PRINT_IMAGES
    //save_img(blurred_img, height, width, "blurred_img.png");
//#endif

    // Second gaussian blur operation. Kernel size is chosen as 5.
    // Applying two consecutive gaussian blur looks a bit odd but this is a requirement of canny edge detection algorithm.
    gaussian_blur(blurred_img, 5, height, width, blurred_img_before_canny);

//#ifdef PRINT_IMAGES
    //save_img(blurred_img_before_canny, height, width, "blurred_img_before_canny.png");
//#endif

    // First operation of sobel. Detects lines in the x direction.
    double flattened_gx[3 * 3] = {-1.0, 0.0, 1.0, -2.0, 0.0, 2.0, -1.0, 0.0, 1.0};

    sobel_g_operator(blurred_img_before_canny, flattened_gx, 3, height, width, gx_canny);

//#ifdef PRINT_IMAGES
    //save_img(gx_canny, height, width, "gx_canny.png");
//#endif

    // Second operation of sobel. Detects lines in the y direction.
    double flattened_gy[3 * 3] = {1.0, 2.0, 1.0, 0.0, 0.0, 0.0, -1.0, -2.0, -1.0};

    sobel_g_operator(blurred_img_before_canny, flattened_gy, 3, height, width, gy_canny);
    
//#ifdef PRINT_IMAGES
    //save_img(gy_canny, height, width, "gy_canny.png");
//#endif

    // Merge of gx and gy frames to generate edge marked frame.
    int lower_threshold = 40;
    int upper_threshold = 50;
    sobel(gx_canny, gy_canny, height, width, 40, 50, sobel_img);

//#ifdef PRINT_IMAGES
    save_img(sobel_img, height, width, "sobel_img.png");
//#endif

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("[Edge detection] Execution is complete... %f\n", time_taken);

    return 0;
}

