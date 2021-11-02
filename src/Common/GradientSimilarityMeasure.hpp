//
//  GradientSimilarityMeasure.hpp
//  gradientSim
//
//  Created by DHR on 2021/10/26.
//

#pragma once
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

namespace VolCorrelation {

#ifndef VolCorrelation_Vec3
#define VolCorrelation_Vec3
template <typename T> struct Vec3 {
  T x, y, z;
  Vec3() : x(0), y(0), z(0) {}
  Vec3(T a, T b, T c) : x(a), y(b), z(c) {}
  auto norm() const -> T { return std::sqrt(x * x + y * y + z * z); }
};
#endif

template <typename ResultType>
auto calculateGradient(const ResultType *field, const Vec3<uint32_t> &pos,
                       const Vec3<uint32_t> dimensions, size_t index,
                       size_t offsetZ) -> Vec3<ResultType> {
  static int kx[3][3][3] = {
      {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}},
      {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
      {{-1, -2, -2}, {-2, -4, -2}, {-1, -2, -1}},
  };

  static int ky[3][3][3] = {
      {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}},
      {{2, 4, 2}, {0, 0, 0}, {-2, -4, -2}},
      {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}},
  };

  static int kz[3][3][3] = {
      {{1, 0, -1}, {2, 0, -2}, {1, 0, -1}},
      {{2, 0, -2}, {4, 0, -4}, {2, 0, -2}},
      {{1, 0, -1}, {2, 0, -2}, {1, 0, -1}},
  };

  const int maxX = dimensions.x;
  const int maxY = dimensions.y;
  const int maxZ = dimensions.z;

  Vec3<ResultType> gradient;

  int x, y, z;
  ResultType valueX, valueY, valueZ;
  for (x = -1; x <= 1; x++) {
    for (y = -1; y <= 1; y++) {
      for (z = -1; z <= 1; z++) {
        if ((pos.x == 0 && x == -1) || (pos.x == maxX - 1 && x == 1)) {
          valueX = field[index];
        } else {
          valueX = field[index + x];
        }
        gradient.x += kx[x + 1][y + 1][z + 1] * valueX;

        if ((pos.y == 0 && y == -1) || (pos.y == maxY - 1 && y == 1)) {
          valueY = field[index];
        } else {
          valueY = field[index + y * maxX];
        }
        gradient.y += ky[x + 1][y + 1][z + 1] * valueY;

        if ((pos.z == 0 && z == -1) || (pos.z == maxZ - 1 && z == 1)) {
          valueZ = field[index];
        } else {
          valueZ = field[index + z * offsetZ];
        }
        gradient.z += kz[x + 1][y + 1][z + 1] * valueZ;
      }
    }
  }
  gradient.x /= 16;
  gradient.y /= 16;
  gradient.z /= 16;

  return gradient;
}

template <typename ResultType>
auto calculatePairSimilarity(const Vec3<ResultType> &gi,
                             const Vec3<ResultType> &gj, int sensitivity)
    -> ResultType {
  if (gi.x == 0 && gi.y == 0 && gi.z == 0) {
    return 0.0;
  }

  if (gj.x == 0 && gj.y == 0 && gj.z == 0) {
    return 0.0;
  }

  if (abs(gi.x - gj.x) < 1e-9 && abs(gi.y - gj.y) < 1e-9 &&
      abs(gi.z - gj.z) < 1e-9) {
    return 1.0;
  }

  auto giNorm = gi.norm();
  auto gjNorm = gj.norm();

  auto tmp = gi.x * gj.x + gi.y * gj.y + gi.z * gj.z;
  auto tmp2 = giNorm + gjNorm;
  auto result = 4 * tmp * tmp / ResultType(giNorm * gjNorm * tmp2 * tmp2);
  return pow(result, sensitivity);
}

template <typename T, typename ResultType = double>
auto calculateGradientSimilarity(const std::vector<T *> fields, uint32_t width,
                                 uint32_t height, uint32_t depth,
                                 int sensitivity = 2)
    -> std::vector<ResultType> {
  using std::vector;

  Vec3<uint32_t> dimensions(width, height, depth);
  auto offsetZ = width * height;
  auto size = depth * offsetZ;

  vector<vector<ResultType>> normalizeds;
  // normalize fields
  for (auto &field : fields) {
    vector<ResultType> normalized(size);
    T max = field[0];
    for (size_t idx = 0; idx < size; idx++) {
      auto value = field[idx];
      if (value > max) {
        max = value;
      }
    }
    for (size_t idx = 0; idx < size; idx++) {
      normalized[idx] = static_cast<ResultType>(field[idx]) / max;
    }
    normalizeds.push_back(std::move(normalized));
  }

  auto result = vector<ResultType>(size, 1.0);

  for (auto i = 0ul; i < fields.size(); i++) {
    for (auto j = i + 1; j < fields.size(); j++) {
      Vec3<uint32_t> pos;
      size_t index = 0;
      auto fieldA = normalizeds[i].data();
      auto fieldB = normalizeds[j].data();
      for (pos.z = 0; pos.z < depth; pos.z++) {
        for (pos.y = 0; pos.y < height; pos.y++) {
          for (pos.x = 0; pos.x < width; pos.x++) {
            // calculate gradients
            auto gi = calculateGradient<ResultType>(fieldA, pos, dimensions,
                                                    index, offsetZ);
            auto gj = calculateGradient<ResultType>(fieldB, pos, dimensions,
                                                    index, offsetZ);
            // calculate similarity
            auto similarity = calculatePairSimilarity(gi, gj, sensitivity);
            const auto exist = result[index];
            result[index] = fmin(exist, similarity);
            index++;
          }
        }
      }
    }
  }

  return result;
}
}
