#pragma once

#include <cmath>
#include <cstddef>
#include <vector>

#include "filters/base_filter.h"
#include "bmp.h"
#include "exceptions.h"

namespace ip {

class GaussianBlurFilter : public Filter {
public:
    static constexpr const char* Name = "blur";
    static constexpr int ParamCount = 1;

    explicit GaussianBlurFilter(float sigma) {
        if (!(sigma > 0.0f)) {
            throw FilterError("blur: sigma must be > 0");
        }
        radius_ = static_cast<int>(std::ceil(3.0f * sigma));
        if (radius_ < 1) radius_ = 1;

        kernel_.resize(static_cast<size_t>(2 * radius_ + 1));
        const float two_sigma_sq = 2.0f * sigma * sigma;
        float sum = 0.0f;
        for (int i = -radius_; i <= radius_; ++i) {
            const float v = std::exp(
                -static_cast<float>(i * i) / two_sigma_sq);
            kernel_[static_cast<size_t>(i + radius_)] = v;
            sum += v;
        }
        for (float& v : kernel_) {
            v /= sum;
        }
    }

    Image Apply(const Image& in) const override {
        Image tmp = HorizontalPass(in);
        return VerticalPass(tmp);
    }

private:
    Image HorizontalPass(const Image& in) const {
        const int width  = static_cast<int>(in.Width());
        const int height = static_cast<int>(in.Height());
        Image out(in.Width(), in.Height());
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float r = 0.0f;
                float g = 0.0f;
                float b = 0.0f;
                for (int k = -radius_; k <= radius_; ++k) {
                    const float w = kernel_[static_cast<size_t>(k + radius_)];
                    const Color& s = in.GetClamped(x + k, y);
                    r += s.r * w;
                    g += s.g * w;
                    b += s.b * w;
                }
                out.At(static_cast<size_t>(x), static_cast<size_t>(y)) =
                    ClampColor({r, g, b});
            }
        }
        return out;
    }

    Image VerticalPass(const Image& in) const {
        const int width  = static_cast<int>(in.Width());
        const int height = static_cast<int>(in.Height());
        Image out(in.Width(), in.Height());
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float r = 0.0f;
                float g = 0.0f;
                float b = 0.0f;
                for (int k = -radius_; k <= radius_; ++k) {
                    const float w = kernel_[static_cast<size_t>(k + radius_)];
                    const Color& s = in.GetClamped(x, y + k);
                    r += s.r * w;
                    g += s.g * w;
                    b += s.b * w;
                }
                out.At(static_cast<size_t>(x), static_cast<size_t>(y)) =
                    ClampColor({r, g, b});
            }
        }
        return out;
    }

    int radius_ = 1;
    std::vector<float> kernel_;
};

}  // namespace ip