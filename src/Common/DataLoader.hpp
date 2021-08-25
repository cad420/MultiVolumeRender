//
// Created by wyz on 2021/8/23.
//

#ifndef MULTIVOLUMERENDER_VOLUMELOADER_HPP
#define MULTIVOLUMERENDER_VOLUMELOADER_HPP

#include "Common/Logger.hpp"
#include "Common/ScalarFieldData.hpp"
#include <fstream>
#include <string>

void SetDimAndTypeByFileName(std::string file_name, ScalarFieldData &scalar_field_data) noexcept(false)
{
    std::vector<std::string> words;
    words.reserve(5);
    std::string word = "";
    for (auto c : file_name)
    {
        if (c != '_' && c != '.')
        {
            word += c;
        }
        else
        {
            words.push_back(word);
            word = "";
        }
    }
    if (words.size() != 5)
    {
        throw std::runtime_error("file name should named as name_x_y_z_type.raw (mouse_256_256_256_uint8.raw)");
    }
    scalar_field_data.x = std::stoul(words[1]);
    scalar_field_data.y = std::stoul(words[2]);
    scalar_field_data.z = std::stoul(words[3]);
    if (words[4] == "uint8")
    {
        scalar_field_data.type = ScalarFieldDataType::UInt8;
    }
    else if (words[4] == "int8")
    {
        scalar_field_data.type = ScalarFieldDataType::Int8;
    }
    else if (words[4] == "uint16")
    {
        scalar_field_data.type = ScalarFieldDataType::UInt16;
    }
    else if (words[4] == "int16")
    {
        scalar_field_data.type = ScalarFieldDataType::Int16;
    }
    else if (words[4] == "uint32")
    {
        scalar_field_data.type = ScalarFieldDataType::UInt32;
    }
    else if (words[4] == "int32")
    {
        scalar_field_data.type = ScalarFieldDataType::Int32;
    }
    else if (words[4] == "float32")
    {
        scalar_field_data.type = ScalarFieldDataType::Float32;
    }
    else if (words[4] == "float64")
    {
        scalar_field_data.type = ScalarFieldDataType::Float64;
    }
    else
    {
        scalar_field_data.type = ScalarFieldDataType::Unknown;
    }
}

template <class Ty> void LoadRawVolumeData(const char *file_name, ScalarFieldData &scalar_field_data) noexcept(false)
{

    std::ifstream in(file_name, std::ios::binary);
    if (!in.is_open())
    {
        throw std::runtime_error("file open failed!");
    }
    in.seekg(0, std::ios::end);
    size_t file_size = in.tellg();
    if (file_size != (size_t)scalar_field_data.x * scalar_field_data.y * scalar_field_data.z * sizeof(Ty))
    {
        throw std::runtime_error("file size not correct!");
    }
    in.seekg(0, std::ios::beg);
    std::vector<Ty> read_data;
    read_data.resize(file_size, 0);
    in.read(reinterpret_cast<char *>(read_data.data()), file_size);
    in.close();

    Ty min_value = std::numeric_limits<Ty>::max();
    Ty max_value = std::numeric_limits<Ty>::min();
    LOG_INFO("Type({0}) max value is {1}, min value is {2}.", typeid(Ty).name(), min_value, max_value);
    auto min_max = std::minmax_element(read_data.cbegin(), read_data.cend());
    min_value = *min_max.first;
    max_value = *min_max.second;
    scalar_field_data.min_scalar = min_value;
    scalar_field_data.max_scalar = max_value;
    LOG_INFO("Read volume data max value is {0}, min value is {1}.", max_value, min_value);
    scalar_field_data.data.resize(file_size, 0);
    for (size_t i = 0; i < scalar_field_data.data.size(); i++)
    {
        scalar_field_data.data[i] = 1.0 * (read_data[i] - min_value) / (max_value - min_value) * 255;
    }
}

void LoadRawVolumeData(const char *file_name, ScalarFieldData &scalar_field_data)
{
    switch (scalar_field_data.type)
    {
    case ScalarFieldDataType::UInt8:
        LoadRawVolumeData<uint8_t>(file_name, scalar_field_data);
        break;
    case ScalarFieldDataType::Int8:
        LoadRawVolumeData<int8_t>(file_name, scalar_field_data);
        break;
    case ScalarFieldDataType::UInt16:
        LoadRawVolumeData<uint16_t>(file_name, scalar_field_data);
        break;
    case ScalarFieldDataType::Int16:
        LoadRawVolumeData<int16_t>(file_name, scalar_field_data);
        break;
    case ScalarFieldDataType::UInt32:
        LoadRawVolumeData<uint32_t>(file_name, scalar_field_data);
        break;
    case ScalarFieldDataType::Int32:
        LoadRawVolumeData<int32_t>(file_name, scalar_field_data);
        break;
    case ScalarFieldDataType::Float32:
        LoadRawVolumeData<float>(file_name, scalar_field_data);
        break;
    case ScalarFieldDataType::Float64:
        LoadRawVolumeData<double>(file_name, scalar_field_data);
        break;
    case ScalarFieldDataType::Unknown:
        break;
    }
}

#endif // MULTIVOLUMERENDER_VOLUMELOADER_HPP
