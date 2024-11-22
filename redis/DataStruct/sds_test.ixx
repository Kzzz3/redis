module;

#include <cassert>
#include <iostream>
#include <cstring>
#include <thread>
#include <vector>
#include <chrono>

export module sds_test;

import sds;

export void run_sds_tests() {
    using namespace std;

    // Test: Create with empty string
    auto test_create_empty = []() {
        const char* data = "";
        size_t len = std::strlen(data);

        sds* obj = sds::create(data, len, len);
        assert(obj != nullptr);
        assert(obj->length() == 0);
        assert(obj->capacity() == 0);
        assert(std::strcmp(obj->buf, data) == 0);

        sds::destroy(obj);
        };

    // Test: Create with maximum length within range
    auto test_create_max_length = []() {
        size_t len = 255; // Maximum for uint8_t
        char data[256];
        std::fill(data, data + len, 'A');
        data[len] = '\0';

        sds* obj = sds::create(data, len, len);
        assert(obj != nullptr);
        assert(obj->length() == len);
        assert(obj->capacity() == len);
        assert(std::strcmp(obj->buf, data) == 0);

        sds::destroy(obj);
        };

    // Test: Append to empty string
    auto test_append_to_empty = []() {
        const char* append_data = "AppendMe";
        size_t append_len = std::strlen(append_data);

        sds* obj = sds::create("", 0, append_len);
        obj = obj->append(append_data, append_len);

        assert(std::strcmp(obj->buf, append_data) == 0);
        assert(obj->length() == append_len);
        assert(obj->capacity() >= append_len);

        sds::destroy(obj);
        };

    // Test: Append beyond capacity
    auto test_append_beyond_capacity = []() {
        const char* data = "Data";
        const char* append_data = "MoreData";
        size_t len = std::strlen(data);
        size_t append_len = std::strlen(append_data);

        sds* obj = sds::create(data, len, len);
        obj = obj->append(append_data, append_len);

        assert(std::strcmp(obj->buf, "DataMoreData") == 0);
        assert(obj->length() == len + append_len);
        assert(obj->capacity() >= len + append_len);

        sds::destroy(obj);
        };

    // Test: Concurrent creation and destruction
    auto test_concurrent_operations = []() {
        const char* data = "ConcurrentTest";
        size_t len = std::strlen(data);

        auto create_destroy = [data, len]() {
            for (int i = 0; i < 1000; ++i) {
                sds* obj = sds::create(data, len, len);
                assert(obj != nullptr);
                sds::destroy(obj);
            }
            };

        std::thread t1(create_destroy);
        std::thread t2(create_destroy);
        t1.join();
        t2.join();
        };

    // Test: Performance - Creating and destroying a large number of objects
    auto test_performance_create_destroy = []() {
        auto start = std::chrono::high_resolution_clock::now();
        const char* data = "PerformanceTest";
        size_t len = std::strlen(data);

        for (int i = 0; i < 100000; ++i) {
            sds* obj = sds::create(data, len, len);
            sds::destroy(obj);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        cout << "Performance Test - Create and Destroy 100000 objects: " << duration << " ms" << endl;
        };

    // Test: Performance - Appending to a string multiple times
    auto test_performance_append = []() {
        auto start = std::chrono::high_resolution_clock::now();
        const char* append_data = "AppendMe";
        size_t append_len = std::strlen(append_data);

        sds* obj = sds::create("", 0, append_len);
        for (int i = 0; i < 100000; ++i) {
            obj = obj->append(append_data, append_len);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        cout << "Performance Test - Append 100000 times: " << duration << " ms" << endl;

        sds::destroy(obj);
        };

    // Test: Large string creation and manipulation
    auto test_large_string = []() {
        size_t len = 1000000; // 1 million characters
        char* data = new char[len + 1];
        std::fill(data, data + len, 'A');
        data[len] = '\0';

        sds* obj = sds::create(data, len, len);
        assert(obj != nullptr);
        assert(obj->length() == len);
        assert(obj->capacity() >= len);

        sds::destroy(obj);
        delete[] data;
        };

    // Run all test cases
    try {
        cout << "Running detailed tests..." << endl;

        test_create_empty();
        cout << "test_create_empty passed" << endl;

        test_create_max_length();
        cout << "test_create_max_length passed" << endl;

        test_append_to_empty();
        cout << "test_append_to_empty passed" << endl;

        test_append_beyond_capacity();
        cout << "test_append_beyond_capacity passed" << endl;

        test_concurrent_operations();
        cout << "test_concurrent_operations passed" << endl;

        test_performance_create_destroy();
        test_performance_append();
        test_large_string();

        cout << "All detailed tests passed!" << endl;
    }
    catch (const std::exception& e) {
        cerr << "Test failed: " << e.what() << endl;
    }
}
