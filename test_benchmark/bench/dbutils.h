#ifndef DB_UTILS
#define DB_UTILS
#include "dbutils.h"
#include <iostream>
#include <chrono>
#include <random>
#include <filesystem>
#include "../src/bufferpool.hpp"



void create_dir(const std::string& path);
void simulateDatabaseOperations(bufferpool& bp, int operations);


class benchmark_result{
public:
    std::string* data; 
    double value; 

    benchmark_result(double value);

    benchmark_result(std::string name,double value);
};

#endif