# lane_detection_parallelization

gt_outputs directory is where all the outputs that should be created reside  
image.png is the input image (Not used at the moment due to some errors, added for visual purposes)  
input.txt is the input for the program  
track.cpp is the main file that is run by the program  
conv.cpp and conv.hpp are the files that stores the FFT based implementation of 2D convolution as well as serial version  

To make serial and test:
```
make serial
make test_serial
```

To make new and test:
```
make new
make test_new
```
