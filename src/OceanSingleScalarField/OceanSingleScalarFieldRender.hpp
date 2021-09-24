//
// Created by wyz on 2021/9/23.
//

#pragma once
#include <Common/TransferFunc.hpp>
#include <Common/OceanData.hpp>
#include <memory>
class OceanSingScalarFieldRenderImpl;
class OceanSingScalarFieldRender{
  public:
    explicit OceanSingScalarFieldRender(int w,int h);

    void SetOceanScalarFieldData(OceanScalarData);

    void SetTransferFunc(TransferFunc);

    void Render();

    ~OceanSingScalarFieldRender();
  private:
    std::unique_ptr<OceanSingScalarFieldRenderImpl> impl;
};