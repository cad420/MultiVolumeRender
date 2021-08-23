include(${PROJECT_SOURCE_DIR}/cmake/utils.cmake)
include(${PROJECT_SOURCE_DIR}/third_party/glfw.cmake)
list(
        APPEND
        MultiVolumeRender_SRCS
        SingleScalarField/SingleScalarFieldRender.cpp
        ScalarAndVectorField/ScalarAndVectorFiledRender.cpp
        MultiScalarField/MultiScalarFieldRender.cpp
        MultiIsoSurface/MultiIsoSurfaceRender.cpp
)
add_library(LibMultiVolumeRenderer ${MultiVolumeRender_SRCS})