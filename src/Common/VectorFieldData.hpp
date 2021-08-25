//
// Created by wyz on 2021/8/23.
//

#ifndef MULTIVOLUMERENDER_VECTORFIELDDATA_HPP
#define MULTIVOLUMERENDER_VECTORFIELDDATA_HPP
#include <vector>
#include <cstdint>
#include <array>
class VectorFieldData{
public:
    std::vector<std::array<float,3>> vertices;
    std::vector<uint32_t> indices;
};

#endif //MULTIVOLUMERENDER_VECTORFIELDDATA_HPP
