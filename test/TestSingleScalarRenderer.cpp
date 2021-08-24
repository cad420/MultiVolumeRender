//
// Created by wyz on 2021/8/23.
//
#include <Common/TransferFunc.hpp>
#include <Common/DataLoader.hpp>
#include <SingleScalarField/SingleScalarFieldRender.hpp>

int main(){
    try{
        ScalarFieldData scalarFieldData;
        const char *file_name = "aneurism_256_256_256_uint8.raw";
        SetDimAndTypeByFileName(file_name, scalarFieldData);
        LoadRawVolumeData(file_name, scalarFieldData);
        SingleScalarFieldRender singleScalarFieldRender(1200, 900);
        singleScalarFieldRender.SetScalarFieldData(std::move(scalarFieldData));
        TransferFunc tf;
        tf.AddTFPoint({0.0, {0.0, 0.0, 0.0, 0.0}});
        tf.AddTFPoint({0.5, {0.9, 0.5, 0.2, 0.0}});
        tf.AddTFPoint({1.0, {1.0, 1.0, 0.2, 1.0}});
        singleScalarFieldRender.SetTransferFunc(tf);
        singleScalarFieldRender.Render();
    }
    catch (const std::exception& err) {
        LOG_ERROR(err.what());
    }

    return 0;
}
