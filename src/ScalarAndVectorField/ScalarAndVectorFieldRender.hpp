//
// Created by wyz on 2021/8/23.
//

#ifndef MULTIVOLUMERENDER_SCALARANDVECTORFIELDRENDER_HPP
#define MULTIVOLUMERENDER_SCALARANDVECTORFIELDRENDER_HPP

#include <Common/ScalarFieldData.hpp>
#include <Common/TransferFunc.hpp>
#include <Common/VectorFieldData.hpp>
#include <memory>

class ScalarAndVectorFieldRenderImpl;

class ScalarAndVectorFieldRender
{
  public:
    ScalarAndVectorFieldRender(int w, int h);

    void SetScalarFieldData(ScalarFieldData);

    void SetVectorFieldDAta(VectorFieldData);

    void SetTransferFunc(TransferFunc);

    void Render();

    ~ScalarAndVectorFieldRender();

  private:
    std::unique_ptr<ScalarAndVectorFieldRenderImpl> impl;
};

#endif // MULTIVOLUMERENDER_SCALARANDVECTORFIELDRENDER_HPP
