#pragma once

#include <cstddef>
#include <vector>

#include "bmp.h"
#include "exceptions.h"

namespace ip {

// Базовый класс

class Filter {
public:
    virtual ~Filter() = default;
    virtual Image Apply(const Image& in) const = 0;
};

// Пиксельные фильтры

class PixelFilter : public Filter {
public:
    Image Apply(const Image& in) const override {
        Image out(in.Width(), in.Height());
        for (size_t y = 0; y < in.Height(); ++y) {
            for (size_t x = 0; x < in.Width(); ++x) {
                out.At(x, y) = TransformPixel(in.At(x, y));
            }
        }
        return out;
    }

protected:
    virtual Color TransformPixel(const Color& c) const = 0;
};

// Матричные фильтры

class MatrixFilter : public Filter {
public:
    Image Apply(const Image& in) const override {
        return Convolve(in);
    }

protected:
    Image Convolve(const Image& in) const {
        const auto& kernel = GetKernel();
        ValidateKernel(kernel);
        const int rows = static_cast<int>(kernel.size());
        const int cols = static_cast<int>(kernel[0].size());
        const int cy = rows / 2;
        const int cx = cols / 2;

        Image out(in.Width(), in.Height());
        for (size_t y = 0; y < in.Height(); ++y) {
            for (size_t x = 0; x < in.Width(); ++x) {
                float r = 0, g = 0, b = 0;
                for (int ky = 0; ky < rows; ++ky) {
                    for (int kx = 0; kx < cols; ++kx) {
                        const float k = kernel[ky][kx];
                        if (k == 0.0f) continue;
                        const Color& s = in.GetClamped(
                            static_cast<int>(x) + kx - cx,
                            static_cast<int>(y) + ky - cy);
                        r += s.r * k;
                        g += s.g * k;
                        b += s.b * k;
                    }
                }
                out.At(x, y) = ClampColor({r, g, b});
            }
        }
        return out;
    }

    virtual const std::vector<std::vector<float>>& GetKernel() const = 0;

private:
    static void ValidateKernel(
        const std::vector<std::vector<float>>& kernel) {
        if (kernel.empty() || kernel.front().empty()) {
            throw FilterError("matrix filter: empty kernel");
        }
        const int cols = static_cast<int>(kernel[0].size());
        for (const auto& row : kernel) {
            if (static_cast<int>(row.size()) != cols) {
                throw FilterError("matrix filter: uneven kernel rows");
            }
        }
    }
};

}  // namespace ip