//
// Created by wyz on 2021/8/23.
//

#ifndef MULTIVOLUMERENDER_MULTIISOSURFACERENDER_HPP
#define MULTIVOLUMERENDER_MULTIISOSURFACERENDER_HPP

#include <Common/OceanData.hpp>
#include <Common/TransferFunc.hpp>
#include <memory>

class OceanMultiIsoSurfaceRenderImpl;

class OceanMultiIsoSurfaceRender
{
  public:
    OceanMultiIsoSurfaceRender(int w, int h);

    void SetOceanScalarFieldData(OceanScalarData, OceanScalarData);

    void SetIsoValueAndColor(double iso_value1, const std::array<double, 4> &color1, double iso_value2,
                             const std::array<double, 4> &color2);

    void Render();

    ~OceanMultiIsoSurfaceRender();

  private:
    std::unique_ptr<OceanMultiIsoSurfaceRenderImpl> impl;
};

#endif // MULTIVOLUMERENDER_MULTIISOSURFACERENDER_HPP
