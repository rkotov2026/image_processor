#pragma once

#include "filters/base_filter.h"
#include "bmp.h"

namespace ip {

class NegativeFilter : public PixelFilter {
public:
    static constexpr const char* Name = "neg";
    static constexpr int ParamCount = 0;

protected:
    Color TransformPixel(const Color& c) const override {
        return {1.0f - c.r, 1.0f - c.g, 1.0f - c.b};
    }
};

}  // namespace ip