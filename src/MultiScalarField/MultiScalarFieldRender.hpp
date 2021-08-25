//
// Created by wyz on 2021/8/23.
//

#ifndef MULTIVOLUMERENDER_MULTISCALARFIELDRENDER_HPP
#define MULTIVOLUMERENDER_MULTISCALARFIELDRENDER_HPP

#include <Common/ScalarFieldData.hpp>
#include <Common/TransferFunc.hpp>
#include <memory>

class MultiScalarFieldRenderImpl;

class MultiScalarFieldRender
{
  public:
    MultiScalarFieldRender(int w, int h);

    void SetScalarFieldData(ScalarFieldData, ScalarFieldData);

    void SetTransferFunc(TransferFunc, TransferFunc);

    void Render();

    ~MultiScalarFieldRender();

  private:
    std::unique_ptr<MultiScalarFieldRenderImpl> impl;
};

#endif // MULTIVOLUMERENDER_MULTISCALARFIELDRENDER_HPP
