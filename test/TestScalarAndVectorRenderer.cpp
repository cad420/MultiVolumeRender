//
// Created by wyz on 2021/8/23.
//
#include <Common/DataLoader.hpp>
#include <Common/TransferFunc.hpp>
#include <ScalarAndVectorField/ScalarAndVectorFieldRender.hpp>

int main()
{
    try
    {
        ScalarFieldData scalarFieldData;
        const char *file_name = "aneurism_256_256_256_uint8.raw";
        SetDimAndTypeByFileName(file_name, scalarFieldData);
        LoadRawVolumeData(file_name, scalarFieldData);

        ScalarAndVectorFieldRender scalarAndVectorFieldRender(1200, 900);
        scalarAndVectorFieldRender.SetScalarFieldData(std::move(scalarFieldData));
        TransferFunc tf;
        tf.AddTFPoint({0.0, {0.0, 0.0, 0.0, 0.0}});
        tf.AddTFPoint({0.5, {0.9, 0.5, 0.2, 0.0}});
        tf.AddTFPoint({1.0, {1.0, 1.0, 0.2, 1.0}});
        scalarAndVectorFieldRender.SetTransferFunc(std::move(tf));
        {
            VectorFieldData vectorFieldData;
            vectorFieldData.vertices = {
                {20.f, 20.f, 20.f},    {40.f, 20.f, 25.f},    {80.f, 20.f, 30.f},   {160.f, 20.f, 35.f},
                {240.f, 20.f, 40.f},   {240.f, 40.f, 45.f},   {240.f, 80.f, 50.f},  {240.f, 160.f, 55.f},
                {240.f, 240.f, 60.f},  {160.f, 240.f, 65.f},  {80.f, 240.f, 70.f},  {40.f, 240.f, 75.f},
                {20.f, 240.f, 80.f},   {20.f, 160.f, 85.f},   {20.f, 80.f, 90.f},   {20.f, 40.f, 95.f},
                {20.f, 20.f, 100.f},   {40.f, 20.f, 110.f},   {80.f, 20.f, 120.f},  {160.f, 20.f, 130.f},
                {240.f, 20.f, 140.f},  {240.f, 40.f, 150.f},  {240.f, 80.f, 160.f}, {240.f, 160.f, 170.f},
                {240.f, 240.f, 180.f}, {160.f, 240.f, 190.f}, {80.f, 240.f, 200.f}, {40.f, 240.f, 210.f},
                {20.f, 240.f, 220.f},  {20.f, 160.f, 230.f},  {20.f, 80.f, 240.f},  {20.f, 40.f, 250.f}};
            vectorFieldData.indices = {0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  7,  7,  8,
                                       8,  9,  9,  10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16,
                                       16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24,
                                       24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31};
            scalarAndVectorFieldRender.SetVectorFieldDAta(vectorFieldData);
        }
        scalarAndVectorFieldRender.Render();
    }
    catch (const std::exception &err)
    {
        LOG_ERROR(err.what());
    }

    return 0;
}
