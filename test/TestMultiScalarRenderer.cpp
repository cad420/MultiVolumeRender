//
// Created by wyz on 2021/8/23.
//
#include <Common/DataLoader.hpp>
#include <Common/TransferFunc.hpp>
#include <MultiScalarField/MultiScalarFieldRender.hpp>

int main()
{
    try
    {
        ScalarFieldData scalarFieldData1;
        const char *file_name = "aneurism_256_256_256_uint8.raw";
        SetDimAndTypeByFileName(file_name, scalarFieldData1);
        LoadRawVolumeData(file_name, scalarFieldData1);
        ScalarFieldData scalarFieldData2;
        file_name = "foot_256_256_256_uint8.raw";
        SetDimAndTypeByFileName(file_name, scalarFieldData2);
        LoadRawVolumeData(file_name, scalarFieldData2);
        MultiScalarFieldRender multiScalarFieldRender(1200, 900);
        multiScalarFieldRender.SetScalarFieldData(std::move(scalarFieldData2), std::move(scalarFieldData1));
        TransferFunc tf1;
        tf1.AddTFPoint({0.0, {0.0, 0.0, 0.0, 0.0}});
        tf1.AddTFPoint({0.5, {0.9, 0.3, 0.2, 0.0}});
        tf1.AddTFPoint({1.0, {1.0, 1.0, 0.2, 1.0}});
        TransferFunc tf2;
        tf2.AddTFPoint({0.0, {0.0, 0.0, 0.0, 0.0}});
        tf2.AddTFPoint({0.5, {0.3, 0.9, 0.2, 0.0}});
        tf2.AddTFPoint({1.0, {1.0, 1.0, 0.2, 1.0}});
        multiScalarFieldRender.SetTransferFunc(std::move(tf1), std::move(tf2));
        multiScalarFieldRender.Render();
    }
    catch (const std::exception &err)
    {
        LOG_ERROR(err.what());
    }

    return 0;
}
