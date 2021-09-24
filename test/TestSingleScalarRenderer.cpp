//
// Created by wyz on 2021/8/23.
//
#include <Common/DataLoader.hpp>
#include <Common/TransferFunc.hpp>
#include <SingleScalarField/SingleScalarFieldRender.hpp>

int main()
{
    try
    {
        ScalarFieldData scalarFieldData;
        const char *file_name = "46#40#9#lod0_512_512_512_uint8.raw";
        SetDimAndTypeByFileName(file_name, scalarFieldData);
        LoadRawVolumeData(file_name, scalarFieldData);
//        scalarFieldData.space_x=0.32f;
//        scalarFieldData.space_y=0.32f;
//        scalarFieldData.space_z=1.f;
        SingleScalarFieldRender singleScalarFieldRender(1200, 900);
        singleScalarFieldRender.SetScalarFieldData(std::move(scalarFieldData));
        TransferFunc tf;
        tf.AddTFPoint({0.0, {0.0, 0.0, 0.0, 0.0}});
        tf.AddTFPoint({25.0/255, {0.0, 0.0, 0.0, 0.0}});
        tf.AddTFPoint({60.0/255, {0.75, 0.5, 1.0, 0.0}});
        tf.AddTFPoint({64.0/255, {0.75, 0.75, 0.75, 0.9}});
        tf.AddTFPoint({224.0/255, {1.0, 0.5, 0.75, 0.9}});
        tf.AddTFPoint({1.0, {1.0, 1.0, 1.0, 1.0}});
        singleScalarFieldRender.SetTransferFunc(std::move(tf));
        singleScalarFieldRender.Render();
    }
    catch (const std::exception &err)
    {
        LOG_ERROR(err.what());
    }

    return 0;
}
