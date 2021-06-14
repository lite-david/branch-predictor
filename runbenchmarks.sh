total_gshare_mpi=0
total_tourn_mpi=0
total_custom_mpi=0
declare -a benchmarks
benchmarks=( int_1.bz2 int_2.bz2 fp_1.bz2 fp_2.bz2 mm_1.bz2 mm_2.bz2 )
for benchmark in "${benchmarks[@]}"
do
  echo "Running $benchmark benchmark.."
  
  output=`bunzip2 -kc traces/$benchmark | ./src/predictor --gshare:13`
  echo -n "gshare:     ";echo $output
  mpi=`echo $output | awk '{print $7;}'`
  total_gshare_mpi=`echo "$total_gshare_mpi + $mpi" | bc`
  
  output=`bunzip2 -kc traces/$benchmark | ./src/predictor --tournament:9:10:10`
  echo -n "tournament: ";echo $output
  mpi=`echo $output | awk '{print $7;}'`
  total_tourn_mpi=`echo "$total_tourn_mpi + $mpi" | bc`
  
  output=`bunzip2 -kc traces/$benchmark | ./src/predictor --tage:12:15:2:7`
  echo -n "tage:      ";echo $output
  mpi=`echo $output | awk '{print $7;}'`
  total_custom_mpi=`echo "$total_custom_mpi + $mpi" | bc`
done
echo -n "Average misprediction for gshare predictor is: "
echo "scale=4; $total_gshare_mpi/6" | bc

echo -n "Average misprediction for tournament predictor is: "
echo "scale=4; $total_tourn_mpi/6" | bc

echo -n "Average misprediction for tage is: "
echo "scale=4; $total_custom_mpi/6" | bc
