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
class OceanScalarData{
  public:
    static constexpr int earth_radius=6371393;//meter
    struct OceanScalarDataPoint{
        double longitude;
        double latitude;
        double distance;//point distance to geocentric
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
    auto GetBoundingVertex(){
        if(data_pts.size()!=num_x*num_y*num_z){
            throw std::runtime_error("number of data points is not correct");
        }
        std::vector<std::array<double,3>> v;
        v.reserve(8*3);
        size_t idx = 0;
        v.push_back({data_pts[idx].longitude,data_pts[idx].latitude,data_pts[idx].distance});
        idx = num_x-1;
        v.push_back({data_pts[idx].longitude,data_pts[idx].latitude,data_pts[idx].distance});
        idx = (num_y-1)*num_x + num_x -1;
        v.push_back({data_pts[idx].longitude,data_pts[idx].latitude,data_pts[idx].distance});
        idx = (num_y-1)*num_x;
        v.push_back({data_pts[idx].longitude,data_pts[idx].latitude,data_pts[idx].distance});

        idx = (num_z-1)*num_x*num_y;
        v.push_back({data_pts[idx].longitude,data_pts[idx].latitude,data_pts[idx].distance});
        idx = (num_z-1)*num_x*num_y + num_x - 1;
        v.push_back({data_pts[idx].longitude,data_pts[idx].latitude,data_pts[idx].distance});
        idx = (num_z-1)*num_x*num_y + (num_y-1)*num_x + num_x - 1;
        v.push_back({data_pts[idx].longitude,data_pts[idx].latitude,data_pts[idx].distance});
        idx = (num_z-1)*num_x*num_y + (num_y-1)*num_x;
        v.push_back({data_pts[idx].longitude,data_pts[idx].latitude,data_pts[idx].distance});

        for(auto& it:v)
            CoordTransform(it);
        return v;
    }
    //long lat dist-> xyz
    static constexpr double Radians(double degree){
        return degree* 3.14159265358979323846/180.0;
    }
    void CoordTransform(std::array<double,3>& coord){
        double y=coord[2]*std::cos(Radians(coord[1]))*std::sin(Radians(coord[0]));
        double z=coord[2]*std::sin(Radians(coord[1]));
        double x=coord[2]*std::cos(Radians(coord[1]))*std::cos(Radians(coord[0]));
        LOG_INFO("dist1 {0}",sqrt(x*x+y*y+z*z));
        coord[0] = x;
        coord[1] = y;
        coord[2] = z;
        LOG_INFO("dist2 {0}",sqrt(float(x)*float(x)+float(y)*float(y)+float(z)*float(z)));
    }

    void GetDataAreaRange(double& min_lon,double& min_lat,double& min_dist,double& len_lon,double& len_lat,double& len_dist){
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
};
