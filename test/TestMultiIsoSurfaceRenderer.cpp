//
// Created by wyz on 2021/8/23.
//
#include <Common/DataLoader.hpp>
#include <Common/TransferFunc.hpp>
#include <MultiIsoSurface/MultiIsoSurfaceRender.hpp>

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
        MultiIsoSurfaceRender multiIsoSurfaceRender(1200, 900);
        multiIsoSurfaceRender.SetScalarFieldData(std::move(scalarFieldData2), std::move(scalarFieldData1));
        double iso_value1 =
            1.0 * (127 - scalarFieldData1.min_scalar) / (scalarFieldData1.max_scalar - scalarFieldData1.min_scalar);
        double iso_value2 =
            1.0 * (200 - scalarFieldData2.min_scalar) / (scalarFieldData2.max_scalar - scalarFieldData2.min_scalar);
        multiIsoSurfaceRender.SetIsoValueAndColor(iso_value1, {1.0, 0.0, 0.0, 1.0}, iso_value2, {0.0, 1.0, 0.0, 1.0});
        multiIsoSurfaceRender.Render();
    }
    catch (const std::exception &err)
    {
        LOG_ERROR(err.what());
    }

    return 0;
}
