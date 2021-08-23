//
// Created by wyz on 2021/8/23.
//

#ifndef MULTIVOLUMERENDER_VOLUMEDATA_HPP
#define MULTIVOLUMERENDER_VOLUMEDATA_HPP
#include <cstdint>
enum class ScalarFieldDataType{
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
    uint32_t x,y,z;
    ScalarFieldDataType type;

};


#endif //MULTIVOLUMERENDER_VOLUMEDATA_HPP
