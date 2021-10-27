//
// Created by wyz on 2021/9/23.
//

#pragma once
#include <Common/TransferFunc.hpp>
#include <Common/OceanData.hpp>
#include <memory>
class OceanMultiScalarFieldRenderImpl;
class OceanMultiScalarFieldRender{
  public:
    explicit OceanMultiScalarFieldRender(int w,int h);

    void SetOceanScalarFieldData(OceanScalarData,OceanScalarData);

    void SetTransferFunc(TransferFunc,TransferFunc);

    void Render();

    ~OceanMultiScalarFieldRender();
  private:
    std::unique_ptr<OceanMultiScalarFieldRenderImpl> impl;
};