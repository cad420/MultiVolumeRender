//
// Created by wyz on 2021/9/23.
//

#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <array>
#include <cmath>
#include <Common/Logger.hpp>
#include <Common/GradientSimilarityMeasure.hpp>
#include <iostream>
#include <fstream>
class OceanScalarData{
  public:
    static constexpr int earth_radius=6371393;//meter
    struct OceanScalarDataPoint{
        float longitude;
        float latitude;
        float distance;//point distance to geocentric
        uint8_t scalar;
    };
    explicit OceanScalarData(int num_x,int num_y,int num_z):num_x(num_x),num_y(num_y),num_z(num_z){
        data_pts.reserve(this->num_x*num_y*num_z);
    }
    void AddOceanScalarPoint(OceanScalarDataPoint const& p){
        data_pts.push_back(p);
    }
    void ReOrder(){
        std::sort(data_pts.begin(),data_pts.end(),[](OceanScalarDataPoint const& p1,OceanScalarDataPoint const& p2){
          if(p1.distance==p2.distance){
              if(p1.latitude==p2.latitude){
                  return p1.longitude<p2.longitude;
              }
              else{
                  return p1.latitude<p2.latitude;
              }
          }
          else{
              return p1.distance<p2.distance;
          }
        });
    }
    auto GetDataArray(){
        std::vector<uint8_t> data_array;
        data_array.reserve(data_pts.size());
        for(auto& it:data_pts)
            data_array.push_back(it.scalar);
        return data_array;
    }
    auto& GetBoundaryVertices(){
        return this->vertices;
    }
    auto& GetBoundaryIndices(){
        return this->indices;
    }
    /**
     * generate data-boundary mesh that has 6 surface not plane
     */
    auto GenerateBoundaryMesh(){
        ReOrder();
        int row,col;
        int triangle_num;
        int pre_index;
        //1.bottom surface
        if(true){
            row = num_y;
            col = num_x;
            triangle_num = (row - 1) * (col - 1) * 2;
            pre_index = this->vertices.size()/3;
            std::vector<float> vertex;
            vertex.reserve(row*col*3);
            for(int i = 0;i<row*col;i++){
                std::array<float,3> v = {data_pts[i].longitude,data_pts[i].latitude,data_pts[i].distance};
                CoordTransform(v);
                vertex.insert(vertex.end(),v.begin(),v.end());
            }
            std::vector<uint32_t> index;
            index.reserve(triangle_num*3);
            for(int i=0;i<row-1;i++){
                for(int j=0;j<col-1;j++){
                    //triangle 1
                    // row i, col j
                    index.push_back(i*col+j+pre_index);
                    // row i+1, col j
                    index.push_back((i+1)*col+j+pre_index);
                    // row i+1, col j+1
                    index.push_back((i+1)*col+j+1+pre_index);


                    //triangle 2
                    // row i, col j
                    index.push_back(i*col+j+pre_index);
                    // row i+1, col j+1
                    index.push_back((i+1)*col+j+1+pre_index);
                    // row i, col j+1
                    index.push_back(i*col+j+1+pre_index);


                }
            }
            assert(index.size() == triangle_num * 3);
            this->vertices.insert(this->vertices.end(),vertex.begin(),vertex.end());
            this->indices.insert(this->indices.end(),index.begin(),index.end());
            std::cout<<"bottom surface triangle num: "<<triangle_num<<std::endl;
        }
        //2.front surface
        if(true){
            row = num_z;
            col = num_x;
            triangle_num = (row - 1) * (col - 1) * 2;
            pre_index = this->vertices.size()/3;
            std::vector<float> vertex;
            vertex.reserve(row*col*3);
            for(int i =0;i<row;i++){
                for(int j =0;j<col;j++){
                    int idx = i*(num_x*num_y)+j;
                    std::array<float,3> v = {data_pts[idx].longitude,data_pts[idx].latitude,data_pts[idx].distance};
                    CoordTransform(v);
                    vertex.insert(vertex.end(),v.begin(),v.end());
                }
            }

            std::vector<uint32_t> index;
            index.reserve(triangle_num*3);
            for(int i=0;i<row-1;i++){
                for(int j=0;j<col-1;j++){
                    //triangle 1
                    // row i, col j
                    index.push_back(i*col+j+pre_index);
                    // row i+1, col j+1
                    index.push_back((i+1)*col+j+1+pre_index);
                    // row i+1, col j
                    index.push_back((i+1)*col+j+pre_index);

                    //triangle 2
                    // row i, col j
                    index.push_back(i*col+j+pre_index);
                    // row i, col j+1
                    index.push_back(i*col+j+1+pre_index);
                    // row i+1, col j+1
                    index.push_back((i+1)*col+j+1+pre_index);

                }
            }
            assert(index.size() == triangle_num * 3);
            this->vertices.insert(this->vertices.end(),vertex.begin(),vertex.end());
            this->indices.insert(this->indices.end(),index.begin(),index.end());
            std::cout<<"front surface triangle num: "<<triangle_num<<std::endl;
        }
        //3.left surface
        if(true){
            row = num_z;
            col = num_y;
            triangle_num = (row - 1) * (col - 1) * 2;
            pre_index = this->vertices.size()/3;
            std::vector<float> vertex;
            vertex.reserve(row*col*3);
            for(int i =0;i<row;i++){
                for(int j =0;j<col;j++){
                    int idx = i*(num_x*num_y)+(col-1-j)*num_x;
                    std::array<float,3> v = {data_pts[idx].longitude,data_pts[idx].latitude,data_pts[idx].distance};
                    CoordTransform(v);
                    vertex.insert(vertex.end(),v.begin(),v.end());
                }
            }

            std::vector<uint32_t> index;
            index.reserve(triangle_num*3);
            for(int i=0;i<row-1;i++){
                for(int j=0;j<col-1;j++){
                    //triangle 1
                    // row i, col j
                    index.push_back(i*col+j+pre_index);
                    // row i+1, col j+1
                    index.push_back((i+1)*col+j+1+pre_index);
                    // row i+1, col j
                    index.push_back((i+1)*col+j+pre_index);

                    //triangle 2
                    // row i, col j
                    index.push_back(i*col+j+pre_index);
                    // row i, col j+1
                    index.push_back(i*col+j+1+pre_index);
                    // row i+1, col j+1
                    index.push_back((i+1)*col+j+1+pre_index);

                }
            }
            assert(index.size() == triangle_num * 3);
            this->vertices.insert(this->vertices.end(),vertex.begin(),vertex.end());
            this->indices.insert(this->indices.end(),index.begin(),index.end());
            std::cout<<"left surface triangle num: "<<triangle_num<<std::endl;
        }
        //4.right surface
        if(true){
            row = num_z;
            col = num_y;
            triangle_num = (row - 1) * (col - 1) * 2;
            pre_index = this->vertices.size()/3;
            std::vector<float> vertex;
            vertex.reserve(row*col*3);
            for(int i =0;i<row;i++){
                for(int j =0;j<col;j++){
                    int idx = i*(num_x*num_y)+(j+1)*num_x-1;
                    std::array<float,3> v = {data_pts[idx].longitude,data_pts[idx].latitude,data_pts[idx].distance};
                    CoordTransform(v);
                    vertex.insert(vertex.end(),v.begin(),v.end());
                }
            }

            std::vector<uint32_t> index;
            index.reserve(triangle_num*3);
            for(int i=0;i<row-1;i++){
                for(int j=0;j<col-1;j++){
                    //triangle 1
                    // row i, col j
                    index.push_back(i*col+j+pre_index);
                    // row i+1, col j+1
                    index.push_back((i+1)*col+j+1+pre_index);
                    // row i+1, col j
                    index.push_back((i+1)*col+j+pre_index);

                    //triangle 2
                    // row i, col j
                    index.push_back(i*col+j+pre_index);
                    // row i, col j+1
                    index.push_back(i*col+j+1+pre_index);
                    // row i+1, col j+1
                    index.push_back((i+1)*col+j+1+pre_index);

                }
            }
            assert(index.size() == triangle_num * 3);
            this->vertices.insert(this->vertices.end(),vertex.begin(),vertex.end());
            this->indices.insert(this->indices.end(),index.begin(),index.end());
            std::cout<<"right surface triangle num: "<<triangle_num<<std::endl;
        }
        //5.back surface
        if(true){
            row = num_z;
            col = num_x;
            triangle_num = (row - 1) * (col - 1) * 2;
            pre_index = this->vertices.size()/3;
            std::vector<float> vertex;
            vertex.reserve(row*col*3);
            for(int i =0;i<row;i++){
                for(int j =0;j<col;j++){
                    int idx = i*(num_x*num_y)+(num_y-1)*num_x+(col-1-j);
                    std::array<float,3> v = {data_pts[idx].longitude,data_pts[idx].latitude,data_pts[idx].distance};
                    CoordTransform(v);
                    vertex.insert(vertex.end(),v.begin(),v.end());
                }
            }

            std::vector<uint32_t> index;
            index.reserve(triangle_num*3);
            for(int i=0;i<row-1;i++){
                for(int j=0;j<col-1;j++){
                    //triangle 1
                    // row i, col j
                    index.push_back(i*col+j+pre_index);
                    // row i+1, col j+1
                    index.push_back((i+1)*col+j+1+pre_index);
                    // row i+1, col j
                    index.push_back((i+1)*col+j+pre_index);

                    //triangle 2
                    // row i, col j
                    index.push_back(i*col+j+pre_index);
                    // row i, col j+1
                    index.push_back(i*col+j+1+pre_index);
                    // row i+1, col j+1
                    index.push_back((i+1)*col+j+1+pre_index);

                }
            }
            assert(index.size() == triangle_num * 3);
            this->vertices.insert(this->vertices.end(),vertex.begin(),vertex.end());
            this->indices.insert(this->indices.end(),index.begin(),index.end());
            std::cout<<"back surface triangle num: "<<triangle_num<<std::endl;
        }
        //6.top surface
        if(true){
            row = num_y;
            col = num_x;
            triangle_num = (row - 1) * (col - 1) * 2;
            pre_index = this->vertices.size()/3;
            std::vector<float> vertex;
            vertex.reserve(row*col*3);
            for(int i =0;i<row;i++){
                for(int j =0;j<col;j++){
                    int idx = (num_z-1)*(num_x*num_y)+i*col+j;
                    std::array<float,3> v = {data_pts[idx].longitude,data_pts[idx].latitude,data_pts[idx].distance};
                    CoordTransform(v);
                    vertex.insert(vertex.end(),v.begin(),v.end());
                }
            }

            std::vector<uint32_t> index;
            index.reserve(triangle_num*3);
            for(int i=0;i<row-1;i++){
                for(int j=0;j<col-1;j++){
                    //triangle 1
                    // row i, col j
                    index.push_back(i*col+j+pre_index);
                    // row i+1, col j+1
                    index.push_back((i+1)*col+j+1+pre_index);
                    // row i+1, col j
                    index.push_back((i+1)*col+j+pre_index);

                    //triangle 2
                    // row i, col j
                    index.push_back(i*col+j+pre_index);
                    // row i, col j+1
                    index.push_back(i*col+j+1+pre_index);
                    // row i+1, col j+1
                    index.push_back((i+1)*col+j+1+pre_index);

                }
            }
            assert(index.size() == triangle_num * 3);
            this->vertices.insert(this->vertices.end(),vertex.begin(),vertex.end());
            this->indices.insert(this->indices.end(),index.begin(),index.end());
            std::cout<<"top surface triangle num: "<<triangle_num<<std::endl;
        }
        std::cout<<"total boundary mesh triangle num: "<<this->indices.size()/3<<std::endl;
        std::cout<<"total boundary mesh vertex num: "<<this->vertices.size()/3<<std::endl;
    }
    //long lat dist-> xyz
    static constexpr double Radians(double degree){
        return degree* 3.14159265358979323846/180.0;
    }
    void CoordTransform(std::array<float,3>& coord){
        float y=coord[2]*std::cos(Radians(coord[1]))*std::sin(Radians(coord[0]));
        float z=coord[2]*std::sin(Radians(coord[1]));
        float x=coord[2]*std::cos(Radians(coord[1]))*std::cos(Radians(coord[0]));
//        LOG_INFO("dist1 {0}",sqrt(x*x+y*y+z*z));
        coord[0] = x;
        coord[1] = y;
        coord[2] = z;
//        LOG_INFO("dist2 {0}",sqrt(float(x)*float(x)+float(y)*float(y)+float(z)*float(z)));
    }

    void GetDataAreaRange(float& min_lon,float& min_lat,float& min_dist,float& len_lon,float& len_lat,float& len_dist){
        if(data_pts.size()!=num_x*num_y*num_z){
            throw std::runtime_error("number of data points is not correct");
        }

        size_t idx = 0;
        min_lon = data_pts[idx].longitude;
        min_lat = data_pts[idx].latitude;
        min_dist = data_pts[idx].distance;

        idx = (num_z-1)*num_x*num_y + (num_y-1)*num_x + num_x - 1;
        len_lon = Radians(data_pts[idx].longitude - min_lon);
        len_lat = Radians(data_pts[idx].latitude - min_lat);
        len_dist = (data_pts[idx].distance - min_dist);
        min_lon = Radians(min_lon);
        min_lat = Radians(min_lat);
//        min_dist /= earth_radius;
    }
    auto GetDataShape() const{
        return std::array<int,3>{(int)num_x,(int)num_y,(int)num_z};
    }
  private:
    size_t num_x,num_y,num_z;
    std::vector<OceanScalarDataPoint> data_pts;
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
};

template <typename T>
auto ReadTxtFile(const std::string& filename)->std::vector<T>{
    std::ifstream in(filename);
    if(!in.is_open()){
        throw std::runtime_error("Open file failed: "+filename);
    }
    std::vector<T> ans;
    float x;
    in.seekg(std::ios::beg);
    while(!in.eof()){
        in >> x;
        ans.push_back(x);
    }
    return ans;
}

template <typename SrcT>
auto TypeTransformToUInt8(const std::vector<SrcT>& src){
    auto min_max = std::minmax_element(src.cbegin(),src.cend());
    SrcT min_val = *min_max.first;
    SrcT max_val = *min_max.second;
    LOG_INFO("min value: {}, max value: {}",min_val,max_val);
    std::vector<uint8_t> res(src.size());
    for(size_t i =0;i<src.size();i++){
        res[i] = 1.0 * (src[i] - min_val) / (max_val-min_val) * 255;
    }
    return res;
}

inline auto ReadOceanDataFromFile(const std::vector<std::string>& paths)->OceanScalarData{
    assert(paths.size()==4);
    if(paths.size()!=4){
        throw std::runtime_error("error paths size");
    }
    auto longtitudes = ReadTxtFile<float>(paths[0]);
    std::cout<<"longtitude size: "<<longtitudes.size()<<std::endl;
    auto latitudes = ReadTxtFile<float>(paths[1]);
    std::cout<<"latitude size: "<<latitudes.size()<<std::endl;
    auto heights = ReadTxtFile<float>(paths[2]);
    std::cout<<"height size: "<<heights.size()<<std::endl;
    auto datas = ReadTxtFile<float>(paths[3]);
    std::cout<<"data size: "<<datas.size()<<std::endl;
    auto count = longtitudes.size()*latitudes.size()*heights.size();
    if(count != datas.size()){
        throw std::runtime_error("data file error");
    }
    auto scalars = TypeTransformToUInt8(datas);
    OceanScalarData ocean_data(longtitudes.size(),latitudes.size(),heights.size());
    int idx = 0;
    for(int d = 0;d<heights.size();d++){
        for(int y=0;y<latitudes.size();y++){
            for(int x=0;x<longtitudes.size();x++){
                ocean_data.AddOceanScalarPoint({longtitudes[x],latitudes[y],6371393.0f+5000*heights[d],scalars[idx++]});
            }
        }
    }
    if(idx != scalars.size()){
        throw std::runtime_error("data size not correct");
    }
    return ocean_data;
}

inline OceanScalarData OceanDataSimilarityMeasure(const std::vector<std::string>& paths_1,const std::vector<std::string>& paths_2){
    assert(paths_1.size()==4 && paths_2.size()==4);
    if(paths_1.size()!=4 || paths_2.size()!=4){
        throw std::runtime_error("error paths size");
    }
    auto longtitudes_1 = ReadTxtFile<float>(paths_1[0]);
    std::cout<<"longtitude size: "<<longtitudes_1.size()<<std::endl;
    auto latitudes_1 = ReadTxtFile<float>(paths_1[1]);
    std::cout<<"latitude size: "<<latitudes_1.size()<<std::endl;
    auto heights_1 = ReadTxtFile<float>(paths_1[2]);
    std::cout<<"height size: "<<heights_1.size()<<std::endl;
    auto datas_1 = ReadTxtFile<float>(paths_1[3]);
    std::cout<<"data size: "<<datas_1.size()<<std::endl;
    auto count_1 = longtitudes_1.size()*latitudes_1.size()*heights_1.size();
    if(count_1 != datas_1.size()){
        throw std::runtime_error("data file error");
    }

    auto longtitudes_2 = ReadTxtFile<float>(paths_2[0]);
    std::cout<<"longtitude size: "<<longtitudes_2.size()<<std::endl;
    auto latitudes_2 = ReadTxtFile<float>(paths_2[1]);
    std::cout<<"latitude size: "<<latitudes_2.size()<<std::endl;
    auto heights_2 = ReadTxtFile<float>(paths_2[2]);
    std::cout<<"height size: "<<heights_2.size()<<std::endl;
    auto datas_2 = ReadTxtFile<float>(paths_2[3]);
    std::cout<<"data size: "<<datas_2.size()<<std::endl;
    auto count_2 = longtitudes_2.size()*latitudes_2.size()*heights_2.size();
    if(count_2 != datas_2.size()){
        throw std::runtime_error("data file error");
    }

    if(latitudes_1.size()!=latitudes_2.size() || longtitudes_1.size()!= longtitudes_2.size() || heights_1.size()!=heights_2.size()
        || datas_1.size()!=datas_2.size()){
        throw std::runtime_error("Two data not the same area");
    }


    using namespace VolCorrelation;
    std::vector<float*> fields = {datas_1.data(),datas_2.data()};
    //calculate gradient similarity before transform to unit8
    auto res = calculateGradientSimilarity(fields,longtitudes_1.size(),latitudes_1.size(),heights_1.size());
    //transform gradient similarity measure result to uint8
    auto scalars = TypeTransformToUInt8(res);

    OceanScalarData ocean_data(longtitudes_1.size(),latitudes_1.size(),heights_1.size());
    int idx = 0;
    for(int d = 0;d<heights_1.size();d++){
        for(int y=0;y<latitudes_1.size();y++){
            for(int x=0;x<longtitudes_1.size();x++){
                ocean_data.AddOceanScalarPoint({longtitudes_1[x],latitudes_1[y],6371393.0f+5000*heights_1[d],scalars[idx++]});
            }
        }
    }
    if(idx != scalars.size()){
        throw std::runtime_error("data size not correct");
    }
    return ocean_data;
}