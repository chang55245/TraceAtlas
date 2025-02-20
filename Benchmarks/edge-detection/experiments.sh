#! /bin/bash

make clean
make serial
make new
make omp
make test_serial
make test_new
make test_omp

rm serial_time.txt
rm pthread_time.txt
rm omp_time.txt

count=$1
for i in $(seq $count); do
    ./lane_detection-serial.native | tail -n 1  >> temp_serial_time.txt
done


grep -Eo '[0-9].[0-9]{1,7}' temp_serial_time.txt >> serial_time.txt

rm temp_serial_time.txt

for i in $(seq $count); do
    ./lane_detection-pthread.native | tail -n 1  >> temp_pthread_time.txt
done

grep -Eo '[0-9].[0-9]{1,7}' temp_pthread_time.txt >> pthread_time.txt
rm temp_pthread_time.txt


for i in $(seq $count); do
    ./lane_detection-omp.native | tail -n 1  >> temp_omp_time.txt
done

grep -Eo '[0-9].[0-9]{1,7}' temp_omp_time.txt >> omp_time.txt
rm temp_omp_time.txt

echo Average of serial  for $1 runs: $(awk '{T+= $NF} END { print T/NR }' serial_time.txt)
echo Average of pthread for $1 runs: $(awk '{T+= $NF} END { print T/NR }' pthread_time.txt)
echo Average of omp     for $1 runs: $(awk '{T+= $NF} END { print T/NR }' omp_time.txt)
