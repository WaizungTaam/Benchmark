benchmark_demo.o: include/benchmark.h test/benchmark_demo.cc
	g++ -std=c++11 include/benchmark.h test/benchmark_demo.cc -o benchmark_demo.o