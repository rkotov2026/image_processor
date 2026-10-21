#pragma once

#include <cstddef>

#include "filters/base_filter.h"
#include "bmp.h"
#include "exceptions.h"

namespace ip {

class BoxBlurFilter : public Filter {
public:
    static constexpr const char* Name = "boxblur";
    static constexpr int ParamCount = 1;   // radius

    explicit BoxBlurFilter(int radius)
        : radius_(radius),
          norm_(1.0f / static_cast<float>(2 * radius + 1)) {
        if (radius_ <= 0) {
            throw FilterError("boxblur: radius must be > 0");
        }
    }

    Image Apply(const Image& in) const override {
        Image tmp = HorizontalPass(in);
        return VerticalPass(tmp);
    }

private:
    // Горизонтальное box-усреднение через скользящее окно: O(n).
    Image HorizontalPass(const Image& in) const {
        const int width  = static_cast<int>(in.Width());
        const int height = static_cast<int>(in.Height());
        const int r = radius_;

        Image out(in.Width(), in.Height());
        for (int y = 0; y < height; ++y) {
            float sr = 0.0f;
            float sg = 0.0f;
            float sb = 0.0f;
            for (int k = -r; k <= r; ++k) {
                const Color& s = in.GetClamped(k, y);
                sr += s.r;
                sg += s.g;
                sb += s.b;
            }
            out.At(0, static_cast<size_t>(y)) =
                ClampColor({sr * norm_, sg * norm_, sb * norm_});

            for (int x = 1; x < width; ++x) {
                const Color& rem = in.GetClamped(x - 1 - r, y);
                const Color& add = in.GetClamped(x + r, y);
                sr += add.r - rem.r;
                sg += add.g - rem.g;
                sb += add.b - rem.b;
                out.At(static_cast<size_t>(x), static_cast<size_t>(y)) =
                    ClampColor({sr * norm_, sg * norm_, sb * norm_});
            }
        }
        return out;
    }

    // Вертикальное box-усреднение через скользящее окно: O(n).
    Image VerticalPass(const Image& in) const {
        const int width  = static_cast<int>(in.Width());
        const int height = static_cast<int>(in.Height());
        const int r = radius_;

        Image out(in.Width(), in.Height());
        for (int x = 0; x < width; ++x) {
            float sr = 0.0f;
            float sg = 0.0f;
            float sb = 0.0f;
            for (int k = -r; k <= r; ++k) {
                const Color& s = in.GetClamped(x, k);
                sr += s.r;
                sg += s.g;
                sb += s.b;
            }
            out.At(static_cast<size_t>(x), 0) =
                ClampColor({sr * norm_, sg * norm_, sb * norm_});

            for (int y = 1; y < height; ++y) {
                const Color& rem = in.GetClamped(x, y - 1 - r);
                const Color& add = in.GetClamped(x, y + r);
                sr += add.r - rem.r;
                sg += add.g - rem.g;
                sb += add.b - rem.b;
                out.At(static_cast<size_t>(x), static_cast<size_t>(y)) =
                    ClampColor({sr * norm_, sg * norm_, sb * norm_});
            }
        }
        return out;
    }

    int radius_;
    float norm_;
};

}  // namespace ip