#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include "exceptions.h"

namespace ip {

// ---------------------------------------------------------------- Color ----

struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
};

// Приводит каждую компоненту к [0, 1].
inline Color ClampColor(const Color& c) {
    return {std::clamp(c.r, 0.0f, 1.0f),
            std::clamp(c.g, 0.0f, 1.0f),
            std::clamp(c.b, 0.0f, 1.0f)};
}

// ---------------------------------------------------------------- Image ----

class Image {
public:
    Image() = default;

    Image(size_t width, size_t height)
        : width_(width), height_(height), pixels_(width * height) {}

    size_t Width() const { return width_; }
    size_t Height() const { return height_; }
    bool Empty() const { return width_ == 0 || height_ == 0; }

    Color& At(size_t x, size_t y) { return pixels_[y * width_ + x]; }

    const Color& At(size_t x, size_t y) const {
        return pixels_[y * width_ + x];
    }

    // Пиксель с «зажимом» координат к границе изображения.
    // Требование задания: за границей использовать ближайший пиксель.
    const Color& GetClamped(int x, int y) const {
        if (width_ == 0 || height_ == 0) {
            throw FilterError("GetClamped on empty image");
        }
        const int w = static_cast<int>(width_);
        const int h = static_cast<int>(height_);
        x = std::clamp(x, 0, w - 1);
        y = std::clamp(y, 0, h - 1);
        return At(static_cast<size_t>(x), static_cast<size_t>(y));
    }

private:
    size_t width_ = 0;
    size_t height_ = 0;
    std::vector<Color> pixels_;
};

// ------------------------------------------------------------- BMP I/O -----

// Читает 24-битный BMP без сжатия с BITMAPINFOHEADER.
// Bottom-up (biHeight > 0) и top-down (biHeight < 0) поддерживаются.
// Бросает BmpError при любой проблеме (нет файла, не BMP, обрезан, ...).
Image ReadBmp(const std::string& path);

// Сохраняет изображение в 24-битный BMP (BITMAPINFOHEADER, bottom-up).
// Бросает BmpError при невозможности записи.
void WriteBmp(const std::string& path, const Image& image);

}  // namespace ip