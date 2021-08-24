//
// Created by wyz on 2021/8/23.
//

#ifndef MULTIVOLUMERENDER_VOLUMEDATA_HPP
#define MULTIVOLUMERENDER_VOLUMEDATA_HPP
#include <cstdint>
#include <vector>
#include <limits>
enum class ScalarFieldDataType{
    Unknown,
    UInt8,
    Int8,
    UInt16,
    Int16,
    UInt32,
    Int32,
    Float32,
    Float64
};

class ScalarFieldData{
public:
    ScalarFieldData():x(0),y(0),z(0),type(ScalarFieldDataType::Unknown){
        max_scalar=-std::numeric_limits<double>::max();
        min_scalar=std::numeric_limits<double>::max();
    }
    uint32_t x,y,z;
    ScalarFieldDataType type;
    std::vector<uint8_t> data;
    double max_scalar,min_scalar;
};


#endif //MULTIVOLUMERENDER_VOLUMEDATA_HPP
