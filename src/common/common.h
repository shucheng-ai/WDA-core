#pragma once
#include <array>
#include <vector>
#include <fstream>
#include <sstream>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <limits>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <algorithm>
#include <sys/stat.h>
#include <glog/logging.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#define EDITING
#define KEYPOINT
#define CHECKPOINT LOG(INFO) << "checkpoint"
#define TESTING raiseError(UnknownError, "debugging");

namespace layout{

    namespace py = pybind11;
    using std::to_string;
    using std::string;
    using std::array;
    using std::vector;
    using std::deque;
    using std::queue;
    using std::list;
    using std::map;
    using std::function;
    using std::unique_ptr;
    using std::shared_ptr;
    using std::numeric_limits;
    using std::tuple;
    using std::hash;
    using std::pair;
    using std::make_pair;
    using namespace pybind11::literals;
    typedef std::pair<int, int> pi;

    class Qlog{
        public :
            Qlog (char const *name){
                google::InitGoogleLogging(name);
            }

            void setLogPath (string const &path){
                FLAGS_stderrthreshold = google::INFO;
                LOG(INFO) << "Set Log Path : " << path;

                google::SetLogDestination(google::INFO, path.c_str());
                FLAGS_max_log_size = 100;
                FLAGS_stop_logging_if_full_disk = true;
            }

            ~Qlog (){
                google::ShutdownGoogleLogging();
                LOG(INFO) << "pop layout module";
            }
    };

    //hash_compile_time
    using hash_t = std::uint64_t;

    constexpr hash_t prime = 0x100000001B3ull;
    constexpr hash_t basis = 0xCBF29CE484222325ull;

    constexpr hash_t hash_compile_time(char const* str, hash_t last_value = basis){
        return *str ? hash_compile_time(str+1, (*str ^ last_value) * prime) : last_value;
    }

    inline constexpr size_t operator "" _hash(char const* p, size_t){
        return hash_compile_time(p);
    }

};

