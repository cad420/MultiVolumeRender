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
        const char *file_name = "foot_256_256_256_uint8.raw";
        SetDimAndTypeByFileName(file_name, scalarFieldData);
        LoadRawVolumeData(file_name, scalarFieldData);
//        scalarFieldData.space_x=0.32f;
//        scalarFieldData.space_y=0.32f;
//        scalarFieldData.space_z=1.f;
        SingleScalarFieldRender singleScalarFieldRender(1200, 900);
        singleScalarFieldRender.SetScalarFieldData(std::move(scalarFieldData));
        TransferFunc tf;
        tf.AddTFPoint({0.0, {1.0, 1.0, 1.0, 0.0}});
        tf.AddTFPoint({0.1, {1.0, 1.0, 1.0, 0.0}});
        //        tf2.AddTFPoint({0.5, {0.3, 0.9, 0.2, 0.0}});
        tf.AddTFPoint({1.0, {0.0, 0.0, 1.0, 0.3}});
        singleScalarFieldRender.SetTransferFunc(std::move(tf));
        singleScalarFieldRender.Render();
    }
    catch (const std::exception &err)
    {
        LOG_ERROR(err.what());
    }

    return 0;
}
