//
// Created by wyz on 2021/8/23.
//

#ifndef MULTIVOLUMERENDER_SINGLESCALARFIELDRENDER_HPP
#define MULTIVOLUMERENDER_SINGLESCALARFIELDRENDER_HPP

#include <Common/ScalarFieldData.hpp>
#include <Common/TransferFunc.hpp>
#include <memory>

class SingleScalarFieldRenderImpl;

class SingleScalarFieldRender
{
  public:
    SingleScalarFieldRender(int w, int h);

    void SetScalarFieldData(ScalarFieldData);

    void SetTransferFunc(TransferFunc);

    void Render();

    ~SingleScalarFieldRender();

  private:
    std::unique_ptr<SingleScalarFieldRenderImpl> impl;
};

#endif // MULTIVOLUMERENDER_SINGLESCALARFIELDRENDER_HPP
