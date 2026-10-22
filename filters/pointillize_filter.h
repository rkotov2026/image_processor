#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>

#include "base_filter.h"
#include "bmp.h"
#include "exceptions.h"

namespace ip {

class PointillizeFilter : public Filter {
public:
    static constexpr const char* Name = "pnt";
    static constexpr int ParamCount = 2;  // radius, seed (0 = random)

    PointillizeFilter(float radius, uint32_t seed)
        : radius_(radius), seed_(seed) {
        if (radius_ <= 0.0f) {
            throw FilterError("pointillize: radius must be > 0");
        }
        if (seed_ == 0) {
            // seed == 0 : случайный seed на этот запуск
            std::random_device rd;
            seed_ = rd();
        }
    }

    Image Apply(const Image& in) const override {
        Image out(in.Width(), in.Height());
        // Белый фон
        for (size_t y = 0; y < out.Height(); ++y) {
            for (size_t x = 0; x < out.Width(); ++x) {
                out.At(x, y) = {1.0f, 1.0f, 1.0f};
            }
        }
        // Локальный ГПСЧ (генератор псевдослучайных чисел) с зафиксированным сидом
        std::mt19937 rng(seed_);
        std::uniform_real_distribution<float> dist(-0.5f, 0.5f);
        const float step = 2.0f * radius_;
        const int cols = static_cast<int>(
            std::ceil(static_cast<float>(in.Width()) / step)) + 1;
        const int rows = static_cast<int>(
            std::ceil(static_cast<float>(in.Height()) / step)) + 1;
        for (int j = 0; j < rows; ++j) {
            for (int i = 0; i < cols; ++i) {
                // Случайное смещение узла в пределах ±radius/2
                const float nx = static_cast<float>(i) * step + dist(rng) * radius_;
                const float ny = static_cast<float>(j) * step + dist(rng) * radius_;
                const Color c = in.GetClamped(static_cast<int>(nx), static_cast<int>(ny));
                DrawDisc(out, nx, ny, radius_, c);
            }
        }
        return out;
    }

private:
    static void DrawDisc(Image& img, float cx, float cy, float r,
                         const Color& c) {
        const int x0 = std::max(0, static_cast<int>(std::floor(cx - r)));
        const int y0 = std::max(0, static_cast<int>(std::floor(cy - r)));
        const int x1 = std::min(static_cast<int>(img.Width())  - 1,
                                static_cast<int>(std::ceil(cx + r)));
        const int y1 = std::min(static_cast<int>(img.Height()) - 1,
                                static_cast<int>(std::ceil(cy + r)));
        const float r2 = r * r;
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                const float dx = static_cast<float>(x) - cx;
                const float dy = static_cast<float>(y) - cy;
                // Принадлежит ли точка кругу
                if (dx * dx + dy * dy <= r2) {
                    img.At(static_cast<size_t>(x), static_cast<size_t>(y)) = c;
                }
            }
        }
    }
    float radius_;
    uint32_t seed_;
};

}  // namespace ip