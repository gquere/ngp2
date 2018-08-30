#!/bin/bash

make clean
make perf
mv ngp_perf ngp_perf_strstr

make clean
make perf ALGO=-D_RK
mv ngp_perf ngp_perf_rk

make clean
make perf ALGO=-D_BMH
mv ngp_perf ngp_perf_bmh


pattern=$1
location=$2


echo "==== GREP ==============================================================="
time grep -rn $pattern $location | wc -l
time grep -r $pattern $location | wc -l

echo "==== STRSTR ============================================================="
time ./ngp_perf_strstr -r $pattern $location
time ./ngp_perf_strstr -r $pattern $location

echo "==== RK ================================================================="
time ./ngp_perf_rk -r $pattern $location
time ./ngp_perf_rk -r $pattern $location

echo "==== BMH ================================================================"
time ./ngp_perf_bmh -r $pattern $location
time ./ngp_perf_bmh -r $pattern $location
