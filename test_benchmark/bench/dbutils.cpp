#include "dbutils.h"
/*#include <iostream>
#include <chrono>
#include <random>
#include <filesystem>
#include "../src/bufferpool.hpp"*/

using namespace std;
using namespace std::chrono;

void create_dir(const std::string& path) {
    std::filesystem::path path_obj(path);
    // check if path exists and is of a regular file
    if (! std::filesystem::exists(path_obj))
        std::filesystem::create_directory(path_obj);
}


// Function to simulate database operations
void simulateDatabaseOperations(bufferpool& bp, int operations) {
    // Random number generator for page ids and operations
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 1000); // Assuming 1000 different pages for simplicity

    int hits = 0;
    for (int i = 0; i < operations; i++) {
        auto page_id = dis(gen); // Generate a random page id
        try {
            // Fetch page and simulate a read or write operation
            auto* pg = bp.fetch_page(page_id);
            if (dis(gen) % 2 == 0) { // Simulate updates 50% of the time
                bp.mark_dirty(page_id);
                memset(pg->payload, 0, PF_PAGE_SIZE); // Simulate modifying the page
            }
            hits++;
        } catch (const exception& e) {
            cerr << "Error during page fetch: " << e.what() << endl;
        }
    }
    cout << "Operations: " << operations << ", Hits: " << hits << ", Hit ratio: " << bp.hit_ratio() << endl;
}


benchmark_result::benchmark_result(double value){ 
    data=nullptr; 
    this->value=value; 
}

benchmark_result::benchmark_result(std::string name,double value){ 
    this->data=new std::string(name); 
    this->value=value; 
}