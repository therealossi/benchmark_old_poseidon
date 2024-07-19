#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <filesystem>
#include <thread>

#include "../src/bufferpool.hpp"
#include "dbutils.h"
#include "benchmarksCreate.h"

using namespace std;
using namespace std::chrono;

void print_benchmark(std::vector<benchmark_result>* bmData, std::ofstream* outFile){
    if (bmData!=nullptr){
        for(benchmark_result res:(*bmData)){
            if(res.data!=nullptr){
                *outFile<<*(res.data)<<":";
            }
            *outFile<<res.value<<"s,";
        }
        *outFile<<std::endl;
    }else{
        *outFile<<"error during benchmark";
    }
    std::cout<<"Benchmark written"<<std::endl;
}

int main(){
    std::cout<<"Start benchmark"<<std::endl;
    std::ofstream outFile("benchmark.txt");

    if (!outFile) {
        std::cerr << "Error opening file for writing." << std::endl;
        return 1;
    }
    std::cout<<"Outfile created"<<std::endl;
    print_benchmark(benchmarkAllocatePage(),&outFile);
    print_benchmark(benchmarkAllocateAndWriteAllPages(),&outFile);
    print_benchmark(benchmarkAllocateWriteDeletePages(),&outFile);
    print_benchmark(benchmarkDeleteAllPages(),&outFile);
    print_benchmark(benchmarkWriteAllPages(),&outFile);

    print_benchmark(benchmarkReadAndDeleteAllPages(),&outFile);
    print_benchmark(benchmarkReadAllPages(),&outFile);
    print_benchmark(benchmarkAllocateWriteReadWriteReadPages(),&outFile);
    print_benchmark(benchmarkAllocateWriteReadDeletePages(),&outFile);
    print_benchmark(benchmarkAllocateWriteReadPages(),&outFile);

    /*auto bmData=benchmarkAllocatePage();
    std::cout<<"Benchmark run"<<std::endl;
    outFile<<"AllocatePage:";
    if (bmData!=nullptr){
        for(benchmark_result res:(*bmData)){
            if(res.data!=nullptr){
                outFile<<*(res.data)<<":";
            }
            outFile<<res.value<<"s,";
        }
        outFile<<std::endl;
    }else{
        outFile<<"error during benchmark";
    }*/
    std::cout<<"Benchmark written"<<std::endl;
    // Close the file
    outFile.close();

    std::cout << "Benchmark run successfully." << std::endl;
    return 0;
}