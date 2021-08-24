//
// Created by wyz on 2021/8/23.
//

#ifndef MULTIVOLUMERENDER_TRANSFERFUNC_HPP
#define MULTIVOLUMERENDER_TRANSFERFUNC_HPP
#include <array>
#include <list>
#include <vector>
#include <map>
#define TF_DIM 256
class TFPoint{
public:
    TFPoint(double key,const std::array<double,4>& value):key(key),value(value){}
    TFPoint(const TFPoint&)=default;
    ~TFPoint()=default;
    bool operator == (const TFPoint& other) const {
        return std::abs(key-other.key)<0.00001;
    }

    double key;//0.0-1.0
    std::array<double,4> value;
};

class TransferFunc{
public:
    TransferFunc()=default;
    void AddTFPoint(const TFPoint& point){ points.push_back(point);}
    void ClearTFPoints(){points.clear();}
    void RemoveTFPoint(const TFPoint& point){
        points.remove(point);
    }
    auto GetColorTable()->const std::vector<float>&;
private:
    std::list<TFPoint> points;
    std::vector<float> transfer_func;
};

inline auto TransferFunc::GetColorTable() -> const std::vector<float>& {
    std::map<uint8_t ,std::array<double,4>> color_setting;
    for(auto& it:points){
        color_setting[it.key*(TF_DIM-1)]=it.value;
    }
    transfer_func.resize(TF_DIM*4,0.f);
    std::vector<uint8_t> keys;
    for(auto it:color_setting)
        keys.emplace_back(it.first);
    size_t size=keys.size();
    for(size_t i=0;i<keys[0];i++){
        transfer_func[i*4+0]=color_setting[keys[0]][0];
        transfer_func[i*4+1]=color_setting[keys[0]][1];
        transfer_func[i*4+2]=color_setting[keys[0]][2];
        transfer_func[i*4+3]=color_setting[keys[0]][3];
    }
    for(size_t i=keys[size-1];i<TF_DIM;i++){
        transfer_func[i*4+0]=color_setting[keys[size-1]][0];
        transfer_func[i*4+1]=color_setting[keys[size-1]][1];
        transfer_func[i*4+2]=color_setting[keys[size-1]][2];
        transfer_func[i*4+3]=color_setting[keys[size-1]][3];
    }
    for(size_t i=1;i<size;i++){
        int left=keys[i-1],right=keys[i];
        auto left_color=color_setting[left];
        auto right_color=color_setting[right];

        for(size_t j=left;j<=right;j++){
            transfer_func[j*4+0]=1.0f*(j-left)/(right-left)*right_color[0]+1.0f*(right-j)/(right-left)*left_color[0];
            transfer_func[j*4+1]=1.0f*(j-left)/(right-left)*right_color[1]+1.0f*(right-j)/(right-left)*left_color[1];
            transfer_func[j*4+2]=1.0f*(j-left)/(right-left)*right_color[2]+1.0f*(right-j)/(right-left)*left_color[2];
            transfer_func[j*4+3]=1.0f*(j-left)/(right-left)*right_color[3]+1.0f*(right-j)/(right-left)*left_color[3];
        }
    }
    return transfer_func;
}

#endif //MULTIVOLUMERENDER_TRANSFERFUNC_HPP
