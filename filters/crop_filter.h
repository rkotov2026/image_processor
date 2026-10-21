#pragma once

#include <algorithm>
#include <cstddef>

#include "filters/base_filter.h"
#include "bmp.h"
#include "exceptions.h"

namespace ip {

class CropFilter : public Filter {
public:
    static constexpr const char* Name = "crop";
    static constexpr int ParamCount = 2;

    CropFilter(size_t width, size_t height) : w_(width), h_(height) {
        if (w_ == 0 || h_ == 0) {
            throw FilterError("crop: width and height must be positive");
        }
    }

    Image Apply(const Image& in) const override {
        const size_t w = std::min(w_, in.Width());
        const size_t h = std::min(h_, in.Height());
        if (w == 0 || h == 0) {
            throw FilterError("crop: image is empty");
        }
        Image out(w, h);
        for (size_t y = 0; y < h; ++y) {
            for (size_t x = 0; x < w; ++x) {
                out.At(x, y) = in.At(x, y);
            }
        }
        return out;
    }

private:
    size_t w_;
    size_t h_;
};

}  // namespace ip