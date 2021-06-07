echo "Running int_1 benchmark.."
bunzip2 -kc traces/int_1.bz2 | ./src/predictor --gshare:13
bunzip2 -kc traces/int_1.bz2 | ./src/predictor --custom

echo "Running int_2 benchmark.."
bunzip2 -kc traces/int_2.bz2 | ./src/predictor --gshare:13
bunzip2 -kc traces/int_2.bz2 | ./src/predictor --custom

echo "Running fp_1 benchmark.."
bunzip2 -kc traces/fp_1.bz2 | ./src/predictor --gshare:13
bunzip2 -kc traces/fp_1.bz2 | ./src/predictor --custom

echo "Running fp_2 benchmark.."
bunzip2 -kc traces/fp_2.bz2 | ./src/predictor --gshare:13
bunzip2 -kc traces/fp_2.bz2 | ./src/predictor --custom

echo "Running mm_1 benchmark.."
bunzip2 -kc traces/mm_1.bz2 | ./src/predictor --gshare:13
bunzip2 -kc traces/mm_1.bz2 | ./src/predictor --custom

echo "Running mm_2 benchmark.."
bunzip2 -kc traces/mm_2.bz2 | ./src/predictor --gshare:13
bunzip2 -kc traces/mm_2.bz2 | ./src/predictor --custom
