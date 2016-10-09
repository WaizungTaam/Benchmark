/*
 * Copyright 2016 Waizung Taam
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* - 2016-10-08
 * - waizungtaam@gmail.com
 */

#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <algorithm>
#include <cstddef>
#include <chrono>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

/*
 * - duration_type_name
 * - measure
 *   - once
 *   - mean
 * - compare
 *   - fastest
 *   - slowest
 *   - sort
 * - report
 *   - time
 *   - comparison
 */

namespace benchmark {

template <typename TimeTp>
std::string duration_type_name() { return "/"; }
template <> std::string duration_type_name<std::chrono::nanoseconds>()
{ return "ns"; }
template <> std::string duration_type_name<std::chrono::microseconds>()
{ return "us"; }
template <> std::string duration_type_name<std::chrono::milliseconds>()
{ return "ms"; }
template <> std::string duration_type_name<std::chrono::seconds>()
{ return "s"; }

template <typename TimeTp>
struct measure {
  template <typename FuncTp, typename ...Args>
  static typename TimeTp::rep once(FuncTp func, Args&&... args) {
    auto begin = std::chrono::system_clock::now();
    func(std::forward<Args>(args)...);
    auto end = std::chrono::system_clock::now();
    auto casted_duration = std::chrono::duration_cast<TimeTp>(end - begin);
    return casted_duration.count();
  }
  template <typename FuncTp, typename ...Args>
  static std::vector<typename TimeTp::rep> once(
      const std::vector<FuncTp>& funcs, Args&&... args) {
    const std::vector<typename TimeTp::rep> durations(funcs.size());
    for (std::size_t i = 0; i < funcs.size(); ++i) {
      durations[i] = once(funcs[i], args...);
    }
    return durations;
  }
  template <typename FuncTp, typename ...Args>
  static std::vector<typename TimeTp::rep> once(
      const std::initializer_list<FuncTp>& funcs, Args&&... args) {
    return once(std::vector<FuncTp>(funcs), args...);
  }

  template <typename FuncTp, typename ...Args>
  static long double mean(std::size_t num_iters, FuncTp func, Args&&... args) {
    auto sum = once(func, args...);
    for (std::size_t i = 1; i < num_iters; ++i) {
      sum += once(func, args...);
    }
    return sum / static_cast<long double>(num_iters);
  }
  template <typename FuncTp, typename ...Args>
  static std::vector<long double> mean(std::size_t num_iters,
      const std::vector<FuncTp>& funcs, Args&&... args) {
    std::vector<long double> durations(funcs.size());
    for (std::size_t i = 0; i < funcs.size(); ++i) {
      durations[i] = mean(num_iters, funcs[i], args...);
    }
    return durations;
  }
  template <typename FuncTp, typename ...Args>
  static std::vector<long double> mean(std::size_t num_iters,
      const std::initializer_list<FuncTp>& funcs, Args&&... args) {
    return mean(num_iters, std::vector<FuncTp>(funcs), args...);
  }  
};

using measure_ns = measure<std::chrono::nanoseconds>;
using measure_us = measure<std::chrono::microseconds>;
using measure_ms = measure<std::chrono::milliseconds>;
using measure_s = measure<std::chrono::seconds>;

struct compare {
  template <typename FuncTp, typename ...Args>
  static std::size_t fastest(std::size_t num_iters, 
      const std::vector<FuncTp>& funcs, Args&& ...args) {
    auto durations = measure_ns::mean(num_iters, funcs, args...);
    auto pos = std::min_element(durations.begin(), durations.end());
    return static_cast<std::size_t>(pos - durations.begin());
  }
  template <typename FuncTp, typename ...Args>
  static std::size_t fastest(std::size_t num_iters, 
      const std::initializer_list<FuncTp>& funcs, Args&& ...args) {
    return fastest(num_iters, std::vector<FuncTp>(funcs), args...);
  }

  template <typename FuncTp, typename ...Args>
  static std::size_t slowest(std::size_t num_iters, 
      const std::vector<FuncTp>& funcs, Args&& ...args) {
    auto durations = measure_ns::mean(num_iters, funcs, args...);
    auto pos = std::max_element(durations.begin(), durations.end());
    return static_cast<std::size_t>(pos - durations.begin());
  }
  template <typename FuncTp, typename ...Args>
  static std::size_t slowest(std::size_t num_iters, 
      const std::initializer_list<FuncTp>& funcs, Args&& ...args) {
    return slowest(num_iters, std::vector<FuncTp>(funcs), args...);
  }

  template <typename FuncTp, typename ...Args>
  static std::vector<std::size_t> sort(std::size_t num_iters, 
      const std::vector<FuncTp>& funcs, Args&& ...args) {
    auto durations = measure_ns::mean(num_iters, funcs, args...);
    // Sort indices according to the durations
    std::map<long double, std::size_t> sorted_map;
    for (std::size_t i = 0; i < funcs.size(); ++i) {
      sorted_map[durations[i]] = i;
    }
    std::vector<std::size_t> sorted_indices;
    for (const auto& p : sorted_map) {
      sorted_indices.push_back(p.second);
    }
    return sorted_indices;
  }
  template <typename FuncTp, typename ...Args>
  static std::vector<std::size_t> sort(std::size_t num_iters, 
      const std::initializer_list<FuncTp>& funcs, Args&& ...args) {
    return sort(num_iters, std::vector<FuncTp>(funcs), args...);
  }  
};

template <typename TimeTp>
struct report {
  template <typename FuncTp, typename CharT, typename Traits, typename ...Args>
  static void time(std::basic_ostream<CharT, Traits>& os, 
      const std::string& name, std::size_t num_iters, 
    FuncTp func, Args&&... args) {
    os << name << ", " << num_iters << ", " 
       << measure<TimeTp>::mean(num_iters, func, args...) << ", "
       << duration_type_name<TimeTp>() << "\n";
  }
  template <typename FuncTp, typename CharT, typename Traits, typename ...Args>
  static void comparison(std::basic_ostream<CharT, Traits>& os,
      const std::vector<std::string>& names, std::size_t num_iters,
      const std::vector<FuncTp>& funcs, Args&&... args) {
    auto durations = measure<TimeTp>::mean(num_iters, funcs, args...);
    // Sort names according to the durations
    std::map<long double, std::string> sorted_map;
    for (std::size_t i = 0; i < durations.size(); ++i) {
      sorted_map[durations[i]] = names[i];
    }
    for (const auto& p : sorted_map) {
       os << p.second << ", " << num_iters << ", " << p.first << ", " 
          << duration_type_name<TimeTp>() << "\n";
    }
  }
  template <typename FuncTp, typename CharT, typename Traits, typename ...Args>
  static void comparison(std::basic_ostream<CharT, Traits>& os,
      const std::initializer_list<std::string>& names, std::size_t num_iters,
      const std::initializer_list<FuncTp>& funcs, Args&&... args) {
    comparison(os, std::vector<std::string>(names), num_iters, 
               std::vector<FuncTp>(funcs), args...);
  }
};

using report_ns = report<std::chrono::nanoseconds>;
using report_us = report<std::chrono::microseconds>;
using report_ms = report<std::chrono::milliseconds>;
using report_s = report<std::chrono::seconds>;

}  // namespace benchmark

#endif  // BENCHMARK_H