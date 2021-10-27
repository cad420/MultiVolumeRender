//
// Created by wyz on 2021/10/27.
//
#include <iostream>
#include <OceanMultiIsoSurface/OceanMultiIsoSurfaceRender.hpp>
int main(){
    try
    {
        OceanMultiIsoSurfaceRender renderer(1200,900);

        OceanScalarData ocean_data1(100,100,52);
        OceanScalarData ocean_data2(100,100,52);

        for(int depth=0;depth<52;depth++){
            for(int height=0;height<100;height++){
                for(int width=0;width<100;width++){
                    ocean_data1.AddOceanScalarPoint({120+width*0.8f,40+height*0.3f,6371393.0f-depth*5000,width<50?uint8_t(255-depth*5):uint8_t(0)});
                    ocean_data2.AddOceanScalarPoint({120+width*0.8f,40+height*0.3f,6371393.0f-depth*5000,width>=50?uint8_t(255-depth*5):uint8_t(0)});
                }
            }
        }
        renderer.SetOceanScalarFieldData(std::move(ocean_data1),std::move(ocean_data2));

        double iso_value1 = 0.5;
        double iso_value2 = 0.7;
        renderer.SetIsoValueAndColor(iso_value1, {1.0, 0.0, 0.0, 1.0}, iso_value2, {0.0, 1.0, 0.0, 1.0});
        renderer.Render();
    }
    catch (const std::exception& err)
    {
        std::cout<<err.what()<<std::endl;
    }
    return 0;
}