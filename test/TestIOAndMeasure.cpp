//
// Created by wyz on 2021/11/15.
//
#include <Common/OceanData.hpp>
#include <OceanSingleScalarField/OceanSingleScalarFieldRender.hpp>

int main(int argc,char** argv){
    try
    {
        //test1: read ocean data from four txt files order by longitude file, latitude file, height file, data file
        {
            auto ocean_data = ReadOceanDataFromFile(
                {{"Windlongtitude.txt"}, {"Windlatitude.txt"}, {"Windheight.txt"}, {"Winddata.txt"}});

            OceanSingScalarFieldRender renderer(1200, 900);

            renderer.SetOceanScalarFieldData(std::move(ocean_data));

            TransferFunc tf;
            tf.AddTFPoint({0.0, {1.0, 1.0, 1.0, 0.0}});
            tf.AddTFPoint({1.0, {1.0, 0.0, 0.0, 1.0}});
            renderer.SetTransferFunc(std::move(tf));

            renderer.Render();
        }
        //test2: read two ocean datas from txt and measure their similarity and then get one OceanData represent similarity
        {
            auto ocean_data = OceanDataSimilarityMeasure(
                {{"Windlongtitude.txt"}, {"Windlatitude.txt"}, {"Windheight.txt"}, {"Winddata.txt"}},
                {{"Windlongtitude.txt"}, {"Windlatitude.txt"}, {"Windheight.txt"}, {"HumidityData.txt"}}
                );
            OceanSingScalarFieldRender renderer(1200, 900);

            renderer.SetOceanScalarFieldData(std::move(ocean_data));

            TransferFunc tf;
            tf.AddTFPoint({0.0, {1.0, 1.0, 1.0, 0.0}});
            tf.AddTFPoint({1.0, {1.0, 0.0, 0.0, 1.0}});
            renderer.SetTransferFunc(std::move(tf));

            renderer.Render();
        }
    }
    catch (std::exception const& err)
    {
        std::cout<<err.what()<<std::endl;
    }

    return 0;
}