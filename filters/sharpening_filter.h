#pragma once

#include <vector>

#include "filters/base_filter.h"
#include "bmp.h"

namespace ip {

class SharpeningFilter : public MatrixFilter {
public:
    static constexpr const char* Name = "sharp";
    static constexpr int ParamCount = 0;

protected:
    const std::vector<std::vector<float>>& GetKernel() const override {
        static const std::vector<std::vector<float>> kKernel = {
            {0.0f, -1.0f, 0.0f},
            {-1.0f, 5.0f, -1.0f},
            {0.0f, -1.0f, 0.0f},
        };
        return kKernel;
    }
};

}  // namespace ip