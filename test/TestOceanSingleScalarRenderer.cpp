//
// Created by wyz on 2021/9/23.
//
#include <Common/Logger.hpp>
#include <OceanSingleScalarField/OceanSingleScalarFieldRender.hpp>
int main(){


    try{
        OceanSingScalarFieldRender renderer(1200,900);

        auto ocean_data = ReadOceanDataFromFile(
            {{"Windlongtitude.txt"}, {"Windlatitude.txt"}, {"Windheight.txt"}, {"Winddata.txt"}});

        renderer.SetOceanScalarFieldData(std::move(ocean_data));

        TransferFunc tf;
        tf.AddTFPoint({0.0, {0.0, 1.0, 0.0, 0.0}});
        tf.AddTFPoint({0.9, {1.0, 1.0, 0.0, 0.0}});
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