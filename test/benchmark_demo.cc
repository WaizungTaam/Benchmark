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

#include "../include/benchmark.h"
#include <iostream>


void f() { for (int i = 0; i < 1000000; ++i) {} }
void g(int x) { for (int i = 0; i < 4000000; ++i) ++x; }
void h(int x, double y) { for (int i = 0; i < 8000000; ++i) y += x; }

void g1(int x) { 
  for (int i = 0; i < 1000000; ++i) 
    x = x % 2 == 0 ? x + 1 : x - 1; 
}
void g2(int x) {
  for (int i = 0; i < 1000000; ++i) 
    x = x % 3 == 0 ? x + 2 : x - 2;
    x = x % 7 == 0 ? x + 3 : x - 3;
}
void g3(int x) { for (int i = 0; i < 1000000; ++i) ++x; }


int main() {
  std::cout << benchmark::measure_ns::once(f) << "\n"
            << benchmark::measure_ns::once(g, 0) << "\n"
            << benchmark::measure_ns::once(h, 0, 0.1) << "\n\n";

  std::cout << benchmark::measure_ns::mean(50, f) << "\n"
            << benchmark::measure_ns::mean(50, g, 0) << "\n"
            << benchmark::measure_ns::mean(50, h, 0, 0.1) << "\n\n";

  benchmark::report_ns::time(std::cout, "g()", 100, g, 1);
  std::cout << "\n";

  benchmark::report_ns::comparison(
    std::cout, {"g1", "g2", "g3"}, 100, {g1, g2, g3}, 1);
  std::cout << "\n";

  std::cout << benchmark::compare::fastest(50, {g1, g2, g3}, 1) << " "
            << benchmark::compare::slowest(50, {g1, g2, g3}, 1) << "\n";

  auto sorted_indices = benchmark::compare::sort(50, {g1, g2, g3}, 1);
  for (const auto& i : sorted_indices) std::cout << i << " ";
  std::cout << "\n";
}