#pragma once

#include <cstddef>
#include <vector>

#include "filters/base_filter.h"
#include "bmp.h"
#include "exceptions.h"
#include "filters/grayscale_filter.h"

namespace ip {

class EdgeDetectionFilter : public MatrixFilter {
public:
    static constexpr const char* Name = "edge";
    static constexpr int ParamCount = 1;

    explicit EdgeDetectionFilter(float threshold) : threshold_(threshold) {
        if (threshold_ < 0.0f || threshold_ > 1.0f) {
            throw FilterError("edge: threshold must be in [0, 1]");
        }
    }

    Image Apply(const Image& in) const override {
        GrayscaleFilter gs;
        Image gray = gs.Apply(in);
        Image conv = Convolve(gray);
        Image out(conv.Width(), conv.Height());
        for (size_t y = 0; y < conv.Height(); ++y) {
            for (size_t x = 0; x < conv.Width(); ++x) {
                const float v = (conv.At(x, y).r > threshold_) ? 1.0f : 0.0f;
                out.At(x, y) = {v, v, v};
            }
        }
        return out;
    }

protected:
    const std::vector<std::vector<float>>& GetKernel() const override {
        static const std::vector<std::vector<float>> kKernel = {
            {0.0f, -1.0f, 0.0f},
            {-1.0f, 4.0f, -1.0f},
            {0.0f, -1.0f, 0.0f},
        };
        return kKernel;
    }

private:
    float threshold_;
};

}  // namespace ip