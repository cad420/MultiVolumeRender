//
// Created by wyz on 2021/8/23.
//

#ifndef MULTIVOLUMERENDER_MULTIISOSURFACERENDER_HPP
#define MULTIVOLUMERENDER_MULTIISOSURFACERENDER_HPP

#include <Common/ScalarFieldData.hpp>
#include <Common/TransferFunc.hpp>
#include <memory>

class MultiIsoSurfaceRenderImpl;

class MultiIsoSurfaceRender
{
  public:
    MultiIsoSurfaceRender(int w, int h);

    void SetScalarFieldData(ScalarFieldData, ScalarFieldData);

    void SetIsoValueAndColor(double iso_value1, const std::array<double, 4> &color1, double iso_value2,
                             const std::array<double, 4> &color2);

    void Render();

    ~MultiIsoSurfaceRender();

  private:
    std::unique_ptr<MultiIsoSurfaceRenderImpl> impl;
};

#endif // MULTIVOLUMERENDER_MULTIISOSURFACERENDER_HPP
