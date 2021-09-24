//
// Created by wyz on 2021/9/23.
//
#include <OceanSingleScalarField/OceanSingleScalarFieldRender.hpp>
#include <Common/Logger.hpp>
int main(){


    try{
        OceanSingScalarFieldRender renderer(1200,900);

        OceanScalarData ocean_data(100,100,52);
//        ocean_data.AddOceanScalarPoint({120.0,45,6261393.0,255});
//        ocean_data.AddOceanScalarPoint({125.0,45,6261393.0,255});
//        ocean_data.AddOceanScalarPoint({120.0,50,6261393.0,255});
//        ocean_data.AddOceanScalarPoint({125.0,50,6261393.0,255});
//        ocean_data.AddOceanScalarPoint({120.0,45,6371393.0,255});
//        ocean_data.AddOceanScalarPoint({125.0,45,6371393.0,255});
//        ocean_data.AddOceanScalarPoint({120.0,50,6371393.0,255});
//        ocean_data.AddOceanScalarPoint({125.0,50,6371393.0,255});
        for(int depth=0;depth<52;depth++){
            for(int height=0;height<100;height++){
                for(int width=0;width<100;width++){
                    ocean_data.AddOceanScalarPoint({120+width*0.8,40+height*0.3,6371393.0-depth*50000,uint8_t(depth*5)});
                }
            }
        }
        renderer.SetOceanScalarFieldData(std::move(ocean_data));

        TransferFunc tf;
        tf.AddTFPoint({0.0, {1.0, 1.0, 1.0, 0.0}});
        tf.AddTFPoint({0.3, {0.0, 0.0, 1.0, 0.0}});
        tf.AddTFPoint({0.6, {0.0, 1.0, 1.0, 1.0}});
        tf.AddTFPoint({0.9, {1.0, 1.0, 0.0, 1.0}});
        tf.AddTFPoint({1.0, {1.0, 0.0, 0.0, 1.0}});
        renderer.SetTransferFunc(std::move(tf));

        renderer.Render();
    }
    catch (const std::exception& err)
    {
        LOG_ERROR("{}",err.what());
    }
    return 0;
}