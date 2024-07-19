#ifndef BENCHMARKS_CREATE
#define BENCHMARKS_CREATE
#include "dbutils.h"
#include <iostream>
#include <chrono>
#include <random>
#include <filesystem>
#include "../src/bufferpool.hpp"

std::vector<benchmark_result>* benchmarkAllocatePage();
std::vector<benchmark_result>* benchmarkAllocateAndWriteAllPages();
std::vector<benchmark_result>* benchmarkAllocateWriteDeletePages();
std::vector<benchmark_result>* benchmarkDeleteAllPages();
std::vector<benchmark_result>* benchmarkAllocateWriteReadPages();

std::vector<benchmark_result>* benchmarkAllocateWriteReadDeletePages();
std::vector<benchmark_result>* benchmarkAllocateWriteReadWriteReadPages();
std::vector<benchmark_result>* benchmarkReadAllPages();
std::vector<benchmark_result>* benchmarkReadAndDeleteAllPages();
std::vector<benchmark_result>* benchmarkWriteAllPages();

#endif