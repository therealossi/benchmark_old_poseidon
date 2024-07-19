#include "benchmarksCreate.h"
#include <random>

void recordTime(std::vector<benchmark_result>* ret, std::chrono::_V2::system_clock::time_point* start, std::string name){
    std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - *start;
    ret->push_back(benchmark_result(name,duration.count()));
    *start=std::chrono::high_resolution_clock::now();
}
// list of benchmarks 
// test1 allocate page
std::vector<benchmark_result>* benchmarkAllocatePage(){
    std::vector<benchmark_result>* ret=new std::vector<benchmark_result>();
    create_dir("benchdir");

    std::cout<<"Created outDir"<<std::endl;

    auto bench_file = std::make_shared<paged_file>();

    std::cout<<"Made file"<<std::endl;

    bench_file->open("benchdir/benches.db", 0);

    std::cout<<"Created dbFile"<<std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    auto mid=std::chrono::high_resolution_clock::now();
    const size_t BUFFER_SIZE = 500; // Adjust based on system capabilities
    bufferpool bp(BUFFER_SIZE);
    bp.register_file(0, bench_file);
    for (auto i = 0u; i < 1000; i++)
            bp.allocate_page(0ul);
    
    recordTime(ret,&mid,"Allocate");

    for (auto i = 1u; i <= 1000; i++) {
        auto p = bp.fetch_page(i);
        p->payload[0] = i+1;
        bp.mark_dirty(i);
    }

    recordTime(ret,&mid,"Initialize");

    bp.flush_all();

    recordTime(ret,&mid,"Flush");

    std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start;
    recordTime(ret,&start,"Total");
    
    std::cout << "Elapsed time: " << duration.count() << "s" << std::endl;
    
    return ret;
}
// test2 allocate and write all pages
std::vector<benchmark_result>* benchmarkAllocateAndWriteAllPages(){
    std::vector<benchmark_result>* ret = new std::vector<benchmark_result>();
    create_dir("benchdir");

    std::cout << "Created outDir" << std::endl;

    auto bench_file = std::make_shared<paged_file>();

    std::cout << "Made file" << std::endl;

    bench_file->open("benchdir/benches.db", 0);

    std::cout << "Created dbFile" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    auto mid = std::chrono::high_resolution_clock::now();
    const size_t BUFFER_SIZE = 500; // Adjust based on system capabilities
    bufferpool bp(BUFFER_SIZE);
    bp.register_file(0, bench_file);
    
    // Allocate pages
    for (auto i = 0u; i < 1000; i++)
        bp.allocate_page(0ul);

    recordTime(ret,&mid,"Allocate");

    // Write to all pages
    for (auto i = 1u; i <= 1000; i++) {
        page* p = bp.fetch_page(i);
        for (size_t j = 0; j < sizeof(p->payload); j++) {
            p->payload[j] = i + j + 1;
        }
        bp.mark_dirty(i);
    }

    recordTime(ret,&mid,"Write");

    bp.flush_all();

    recordTime(ret,&mid,"Flush");

    std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start;
    recordTime(ret,&start,"Total");
    std::cout << "Elapsed time: " << duration.count() << "s" << std::endl;

    return ret;
}
// test3 allocate, write with random input, delete all pages
std::vector<benchmark_result>* benchmarkAllocateWriteDeletePages(){
    std::vector<benchmark_result>* ret = new std::vector<benchmark_result>();
    create_dir("benchdir");

    std::cout << "Created outDir" << std::endl;

    auto bench_file = std::make_shared<paged_file>();

    std::cout << "Made file" << std::endl;

    bench_file->open("benchdir/benches.db", 0);

    std::cout << "Created dbFile" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    auto mid = std::chrono::high_resolution_clock::now();
    const size_t BUFFER_SIZE = 500; // Adjust based on system capabilities
    bufferpool bp(BUFFER_SIZE);
    bp.register_file(0, bench_file);
    
    // Allocate pages
    for (auto i = 0u; i < 1000; i++)
        bp.allocate_page(0ul);

    recordTime(ret,&mid,"Allocate");

    // Write random input to all pages
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10000);

    for (auto i = 1u; i <= 1000; i++) {
        auto p = bp.fetch_page(i);
        for (size_t j = 0; j < sizeof(p->payload); j++) {
            p->payload[j] = dis(gen);
        }
        bp.mark_dirty(i);
    }

    recordTime(ret,&mid,"Write");

    bp.flush_all();
    
    recordTime(ret,&mid,"Flush");
    

    // Delete all pages
    for (auto i = 1u; i <= 1000; i++) {
        bp.free_page(i);
    }

    recordTime(ret,&mid,"Delete");

    std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start;
    recordTime(ret,&start,"Total");
    std::cout << "Elapsed time: " << duration.count() << "s" << std::endl;

    return ret;
}
// test4 delete all pages 
std::vector<benchmark_result>* benchmarkDeleteAllPages(){
    std::vector<benchmark_result>* ret = new std::vector<benchmark_result>();
    create_dir("benchdir");

    std::cout << "Created outDir" << std::endl;

    auto bench_file = std::make_shared<paged_file>();

    std::cout << "Made file" << std::endl;

    bench_file->open("benchdir/benches.db", 0);

    std::cout << "Created dbFile" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    auto mid = std::chrono::high_resolution_clock::now();
    const size_t BUFFER_SIZE = 500; // Adjust based on system capabilities
    bufferpool bp(BUFFER_SIZE);
    bp.register_file(0, bench_file);
    
    // Allocate pages
    for(auto i = 0u; i < 1000; i++){
        bp.allocate_page(0ul);
    }

    recordTime(ret,&mid,"Allocate");
    
     // Delete all pages
    for (auto i = 1u; i <= 1000; i++) {
        bp.free_page(i);
    }

    recordTime(ret,&mid,"Delete");

    std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start;
    recordTime(ret,&start,"Total");
    std::cout << "Elapsed time: " << duration.count() << "s" << std::endl;

    return ret;
}
// test5 allocate, write with random input, read all pages
std::vector<benchmark_result>* benchmarkAllocateWriteReadPages(){
    std::vector<benchmark_result>* ret = new std::vector<benchmark_result>();
    create_dir("benchdir");

    std::cout << "Created outDir" << std::endl;

    auto bench_file = std::make_shared<paged_file>();

    std::cout << "Made file" << std::endl;

    bench_file->open("benchdir/benches.db", 0);

    std::cout << "Created dbFile" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    auto mid = std::chrono::high_resolution_clock::now();
    const size_t BUFFER_SIZE = 500; // Adjust based on system capabilities
    bufferpool bp(BUFFER_SIZE);
    bp.register_file(0, bench_file);

    // Allocate pages
    for (auto i = 0u; i < 1000; i++)
        bp.allocate_page(0ul);

    recordTime(ret,&mid,"Allocate");

    // Write random input to all pages
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10000);

    for (auto i = 1u; i <= 1000; i++) {
        auto p = bp.fetch_page(i);
        for (size_t j = 0; j < sizeof(p->payload); j++) {
            p->payload[j] = dis(gen);
        }
        bp.mark_dirty(i);
    }

    recordTime(ret,&mid,"Write");

    bp.flush_all();

    recordTime(ret,&mid,"Flush");
    
    // Read all pages
    for (auto i = 1u; i <= 1000; i++) {
        auto p = bp.fetch_page(i);
        volatile int read_val = p->payload[0]; // Simulate read operation
        (void)read_val; // Avoid unused variable warning
    }

    recordTime(ret,&mid,"Read");

    std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start;
    recordTime(ret,&start,"Total");
    std::cout << "Elapsed time: " << duration.count() << "s" << std::endl;

    return ret;

}
// test6 allocate, write with random input, read all pages, delete all pages
std::vector<benchmark_result>* benchmarkAllocateWriteReadDeletePages(){
    std::vector<benchmark_result>* ret = new std::vector<benchmark_result>();
    create_dir("benchdir");

    std::cout << "Created outDir" << std::endl;

    auto bench_file = std::make_shared<paged_file>();

    std::cout << "Made file" << std::endl;

    bench_file->open("benchdir/benches.db", 0);

    std::cout << "Created dbFile" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    auto mid = std::chrono::high_resolution_clock::now();
    const size_t BUFFER_SIZE = 500; // Adjust based on system capabilities
    bufferpool bp(BUFFER_SIZE);
    bp.register_file(0, bench_file);
    
    // Allocate pages
    for (auto i = 0u; i < 1000; i++)
        bp.allocate_page(0ul);

    recordTime(ret,&mid,"Allocate");

    // Write random input to all pages
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10000);

    for (auto i = 1u; i <= 1000; i++) {
        auto p = bp.fetch_page(i);
        for (size_t j = 0; j < sizeof(p->payload); j++) {
            p->payload[j] = dis(gen);
        }
        bp.mark_dirty(i);
    }

    recordTime(ret,&mid,"Write");

    bp.flush_all();
    recordTime(ret,&mid,"Flush");

    // Read all pages
    for (auto i = 1u; i <= 1000; i++) {
        auto p = bp.fetch_page(i);
        volatile int read_val = p->payload[0]; // Simulate read operation
        (void)read_val; // Avoid unused variable warning
    }

    recordTime(ret,&mid,"Read");

    // Delete all pages
    for (auto i = 1u; i <= 1000; i++) {
        bp.free_page(i);
    }

    recordTime(ret,&mid,"Delete");

    std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start;
    recordTime(ret,&start,"Total");
    std::cout << "Elapsed time: " << duration.count() << "s" << std::endl;

    return ret;
}
// test7 allocate, write with random input, read all pages, wirte all pages, read all pages
std::vector<benchmark_result>* benchmarkAllocateWriteReadWriteReadPages(){
    std::vector<benchmark_result>* ret = new std::vector<benchmark_result>();
    create_dir("benchdir");

    std::cout << "Created outDir" << std::endl;

    auto bench_file = std::make_shared<paged_file>();

    std::cout << "Made file" << std::endl;

    bench_file->open("benchdir/benches.db", 0);

    std::cout << "Created dbFile" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    auto mid = std::chrono::high_resolution_clock::now();
    const size_t BUFFER_SIZE = 500; // Adjust based on system capabilities
    bufferpool bp(BUFFER_SIZE);
    bp.register_file(0, bench_file);
    
    // Allocate pages
    for (auto i = 0u; i < 1000; i++)
        bp.allocate_page(0ul);

    recordTime(ret,&mid,"Allocate");

    // Write random input to all pages
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10000);

    for (auto i = 1u; i <= 1000; i++) {
        auto p = bp.fetch_page(i);
        for (size_t j = 0; j < sizeof(p->payload); j++) {
            p->payload[j] = dis(gen);
        }
        bp.mark_dirty(i);
    }

    recordTime(ret,&mid,"First write");

    bp.flush_all();

    recordTime(ret,&mid,"Flush after first write");

    // Read all pages
    for (auto i = 1u; i <= 1000; i++) {
        auto p = bp.fetch_page(i);
        volatile int read_val = p->payload[0]; // Simulate read operation
        (void)read_val; // Avoid unused variable warning
    }

    recordTime(ret,&mid,"First read");
    // Write random input to all pages again
    for (auto i = 1u; i <= 1000; i++) {
        auto p = bp.fetch_page(i);
        for (size_t j = 0; j < sizeof(p->payload); j++) {
            p->payload[j] = dis(gen);
        }
        bp.mark_dirty(i);
    }

    recordTime(ret,&mid,"Second write");

    bp.flush_all();

    recordTime(ret,&mid,"Flush after second read");

    // Read all pages again
    for (auto i = 1u; i <= 1000; i++) {
        auto p = bp.fetch_page(i);
        volatile int read_val = p->payload[0]; // Simulate read operation
        (void)read_val; // Avoid unused variable warning
    }
    
    recordTime(ret,&mid,"Second read");
    
    std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start;
    recordTime(ret,&start,"Total");
    std::cout << "Elapsed time: " << duration.count() << "s" << std::endl;

    return ret;
}
// test8 read all files
std::vector<benchmark_result>* benchmarkReadAllPages(){
    std::vector<benchmark_result>* ret = new std::vector<benchmark_result>();
    create_dir("benchdir");

    std::cout << "Created outDir" << std::endl;

    auto bench_file = std::make_shared<paged_file>();

    std::cout << "Made file" << std::endl;

    bench_file->open("benchdir/benches.db", 0);

    std::cout << "Created dbFile" << std::endl;

    const size_t BUFFER_SIZE = 500; // Adjust based on system capabilities
    bufferpool bp(BUFFER_SIZE);
    bp.register_file(0, bench_file);

    // Allocate pages
    for (auto i = 0u; i < 1000; i++)
        bp.allocate_page(0ul);
        
    // Write random input to all pages
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10000);

    for (auto i = 1u; i <= 1000; i++) {
        auto p = bp.fetch_page(i);
        for (size_t j = 0; j < sizeof(p->payload); j++) {
            p->payload[j] = dis(gen);
        }
        bp.mark_dirty(i);
    }
    bp.flush_all();


    auto start = std::chrono::high_resolution_clock::now();
    auto mid = std::chrono::high_resolution_clock::now();

    // Read all pages
    for (auto i = 1u; i <= 1000; i++) {
        auto p = bp.fetch_page(i);
        volatile int read_val = p->payload[0]; // Simulate read operation
        (void)read_val; // Avoid unused variable warning
    }

    recordTime(ret,&mid,"Read");

    std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start;
    recordTime(ret,&start,"Total");
    std::cout << "Elapsed time: " << duration.count() << "s" << std::endl;

    return ret;
}
// test9 read all files, delete all files
std::vector<benchmark_result>* benchmarkReadAndDeleteAllPages(){
    std::vector<benchmark_result>* ret = new std::vector<benchmark_result>();
    create_dir("benchdir");

    std::cout << "Created outDir" << std::endl;

    auto bench_file = std::make_shared<paged_file>();

    std::cout << "Made file" << std::endl;

    bench_file->open("benchdir/benches.db", 0);

    std::cout << "Created dbFile" << std::endl;

    const size_t BUFFER_SIZE = 500; // Adjust based on system capabilities
    bufferpool bp(BUFFER_SIZE);
    bp.register_file(0, bench_file);

    // Allocate pages
    for (auto i = 0u; i < 1000; i++)
        bp.allocate_page(0ul);

    auto start = std::chrono::high_resolution_clock::now();
    auto mid = std::chrono::high_resolution_clock::now();

    // Read all pages
    for (auto i = 1u; i <= 1000; i++) {
        auto p = bp.fetch_page(i);
        volatile int read_val = p->payload[0]; // Simulate read operation
        (void)read_val; // Avoid unused variable warning
    }
    recordTime(ret,&mid,"Read");

    // Delete all pages
    for (auto i = 1u; i <= 1000; i++) {
        bp.free_page(i);
    }

    recordTime(ret,&mid,"Delete");

    std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start;
    recordTime(ret,&start,"Total");
    std::cout << "Elapsed time: " << duration.count() << "s" << std::endl;

    return ret;
}
// test10 write all files 
std::vector<benchmark_result>* benchmarkWriteAllPages(){
    std::vector<benchmark_result>* ret = new std::vector<benchmark_result>();
    create_dir("benchdir");

    std::cout << "Created outDir" << std::endl;

    auto bench_file = std::make_shared<paged_file>();

    std::cout << "Made file" << std::endl;

    bench_file->open("benchdir/benches.db", 0);

    std::cout << "Created dbFile" << std::endl;

    const size_t BUFFER_SIZE = 500; // Adjust based on system capabilities
    bufferpool bp(BUFFER_SIZE);
    bp.register_file(0, bench_file);
    // Allocate pages
    for (auto i = 0u; i < 1000; i++)
        bp.allocate_page(0ul);

    auto start = std::chrono::high_resolution_clock::now();
    auto mid = std::chrono::high_resolution_clock::now();

    // Write random input to all pages
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10000);

    for (auto i = 1u; i <= 1000; i++) {
        auto p = bp.fetch_page(i);
        for (size_t j = 0; j < sizeof(p->payload); j++) {
            p->payload[j] = dis(gen);
        }
        bp.mark_dirty(i);
    }
    recordTime(ret,&mid,"Write");
    
    bp.flush_all();

    recordTime(ret,&mid,"Flush");

    std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start;
    recordTime(ret,&start,"Total");
    std::cout << "Elapsed time: " << duration.count() << "s" << std::endl;

    return ret;
}
