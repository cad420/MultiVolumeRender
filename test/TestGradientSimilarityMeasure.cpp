//
//  main.cpp
//  gradientSim
//
//  Created by DHR on 2021/10/26.
//
#include "Common/GradientSimilarityMeasure.hpp"
#include <iostream>
#include <vector>
int main(int argc, const char * argv[]) {
    std::vector<double> A(100, 1.0);
    std::vector<double> B(100, 0.0);

    std::vector<double *> fields = {A.data(), B.data()};
    const auto res1 = VolCorrelation::calculateGradientSimilarity(fields, 10, 10, 1);
    for (auto v : res1) {
      std::cout << v << " ";
    }
    
//    std::vector<float> VolCorrelation::calculateGradientSimilarity(
//      const std::vector<T *> fields,
//      uint32_t width,
//      uint32_t height,
//      uint32_t depth,
//      int sensitivity = 2
//    );
    return 0;
}
