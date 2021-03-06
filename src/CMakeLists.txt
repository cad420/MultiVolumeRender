include(${PROJECT_SOURCE_DIR}/cmake/utils.cmake)
include(${PROJECT_SOURCE_DIR}/third_party/glm.cmake)
include(${PROJECT_SOURCE_DIR}/third_party/glfw.cmake)
include(${PROJECT_SOURCE_DIR}/third_party/spdlog.cmake)
set(GLAD_ROOT_DIR ${PROJECT_SOURCE_DIR}/third_party/glad4_0)
list(
        APPEND
        MultiVolumeRender_SRCS
        SingleScalarField/SingleScalarFieldRender.cpp
        ScalarAndVectorField/ScalarAndVectorFieldRender.cpp
        MultiScalarField/MultiScalarFieldRender.cpp
        MultiIsoSurface/MultiIsoSurfaceRender.cpp
        OceanSingleScalarField/OceanSingleScalarFieldRender.cpp
        OceanMultiScalarField/OceanMultiScalarFieldRender.cpp
        OceanMultiIsoSurface/OceanMultiIsoSurfaceRender.cpp
        ${GLAD_ROOT_DIR}/src/glad.c
)
add_library(LibMultiVolumeRenderer ${MultiVolumeRender_SRCS})

target_link_libraries(LibMultiVolumeRenderer
        PUBLIC
        spdlog::spdlog
        PRIVATE
        glfw
        glm::glm
        )

target_include_directories(LibMultiVolumeRenderer
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${GLAD_ROOT_DIR}/include
        )
embed_shaders(
        CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/SingleScalarField/Shaders.hpp.in
        TARGET ${CMAKE_CURRENT_SOURCE_DIR}/SingleScalarField/Shaders.hpp
        DIR ${CMAKE_CURRENT_SOURCE_DIR}/SingleScalarField/Shader
)
embed_shaders(
        CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/MultiScalarField/Shaders.hpp.in
        TARGET ${CMAKE_CURRENT_SOURCE_DIR}/MultiScalarField/Shaders.hpp
        DIR ${CMAKE_CURRENT_SOURCE_DIR}/MultiScalarField/Shader
)
embed_shaders(
        CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/MultiIsoSurface/Shaders.hpp.in
        TARGET ${CMAKE_CURRENT_SOURCE_DIR}/MultiIsoSurface/Shaders.hpp
        DIR ${CMAKE_CURRENT_SOURCE_DIR}/MultiIsoSurface/Shader
)
embed_shaders(
        CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/ScalarAndVectorField/Shaders.hpp.in
        TARGET ${CMAKE_CURRENT_SOURCE_DIR}/ScalarAndVectorField/Shaders.hpp
        DIR ${CMAKE_CURRENT_SOURCE_DIR}/ScalarAndVectorField/Shader
)