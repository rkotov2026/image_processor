#pragma once

#include "filters/base_filter.h"
#include "bmp.h"

namespace ip {

class GrayscaleFilter : public PixelFilter {
public:
    static constexpr const char* Name = "gs";
    static constexpr int ParamCount = 0;

protected:
    Color TransformPixel(const Color& c) const override {
        const float v = 0.299f * c.r + 0.587f * c.g + 0.114f * c.b;
        return {v, v, v};
    }
};

}  // namespace ip