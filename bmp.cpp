#include "bmp.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <string>
#include <type_traits>
#include <vector>

#include "exceptions.h"

namespace ip {

namespace {

constexpr uint16_t kSignature = 0x4D42;  // "BM"
constexpr uint32_t kFileHeaderSize = 14;
constexpr uint32_t kInfoHeaderSize = 40;
constexpr uint32_t kHeaderTotal = kFileHeaderSize + kInfoHeaderSize;
constexpr uint16_t kBitCount24 = 24;
constexpr uint32_t kCompressionNone = 0;

uint32_t RowSize(size_t width) {
    // Строка выровнена по 4 байта.
    return static_cast<uint32_t>(((width * 3u + 3u) / 4u) * 4u);
}

template <typename T>
T ReadLE(std::istream& in) {
    static_assert(std::is_unsigned_v<T>, "only unsigned");
    unsigned char bytes[sizeof(T)] = {};
    if (!in.read(reinterpret_cast<char*>(bytes), sizeof(T))) {
        throw BmpError("unexpected end of file");
    }
    uint64_t value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<uint64_t>(bytes[i]) << (8 * i);
    }
    return static_cast<T>(value);
}

template <typename T>
void WriteLE(std::ostream& out, T value) {
    static_assert(std::is_unsigned_v<T>, "only unsigned");
    unsigned char bytes[sizeof(T)] = {};
    for (size_t i = 0; i < sizeof(T); ++i) {
        bytes[i] = static_cast<unsigned char>((value >> (8 * i)) & 0xFFu);
    }
    out.write(reinterpret_cast<const char*>(bytes), sizeof(T));
}

uint8_t ToByte(float v) {
    v = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
    return static_cast<uint8_t>(std::lround(v * 255.0f));
}

}  // namespace

Image ReadBmp(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw BmpError("cannot open input file: " + path);
    }

    // --- BITMAPFILEHEADER ---
    const uint16_t bfType = ReadLE<uint16_t>(in);
    if (bfType != kSignature) {
        throw BmpError("invalid signature (not 'BM')");
    }
    (void)ReadLE<uint32_t>(in);  // bfSize
    (void)ReadLE<uint16_t>(in);  // bfReserved1
    (void)ReadLE<uint16_t>(in);  // bfReserved2
    const uint32_t bfOffBits = ReadLE<uint32_t>(in);

    // --- BITMAPINFOHEADER ---
    const uint32_t biSize = ReadLE<uint32_t>(in);
    if (biSize != kInfoHeaderSize) {
        throw BmpError("only BITMAPINFOHEADER (40 bytes) supported");
    }
    const int32_t biWidth = static_cast<int32_t>(ReadLE<uint32_t>(in));
    const int32_t biHeightRaw = static_cast<int32_t>(ReadLE<uint32_t>(in));
    const uint16_t biPlanes = ReadLE<uint16_t>(in);
    const uint16_t biBitCount = ReadLE<uint16_t>(in);
    const uint32_t biCompression = ReadLE<uint32_t>(in);
    (void)ReadLE<uint32_t>(in);  // biSizeImage
    (void)ReadLE<uint32_t>(in);  // biXPelsPerMeter
    (void)ReadLE<uint32_t>(in);  // biYPelsPerMeter
    (void)ReadLE<uint32_t>(in);  // biClrUsed
    (void)ReadLE<uint32_t>(in);  // biClrImportant

    if (biPlanes != 1) {
        throw BmpError("biPlanes != 1");
    }
    if (biBitCount != kBitCount24) {
        throw BmpError("only 24-bit images supported");
    }
    if (biCompression != kCompressionNone) {
        throw BmpError("compressed BMPs not supported");
    }
    if (biWidth <= 0 || biHeightRaw == 0) {
        throw BmpError("invalid dimensions");
    }

    const bool topDown = biHeightRaw < 0;
    const size_t width = static_cast<size_t>(biWidth);
    const size_t height =
        static_cast<size_t>(topDown ? -biHeightRaw : biHeightRaw);
    const uint32_t rowSize = RowSize(width);

    // Проверим, что файл не обрезан.
    in.seekg(0, std::ios::end);
    const auto endPos = in.tellg();
    if (endPos < 0) {
        throw BmpError("cannot determine file size");
    }
    const uint64_t expectedEnd =
        static_cast<uint64_t>(bfOffBits) +
        static_cast<uint64_t>(rowSize) * height;
    if (static_cast<uint64_t>(endPos) < expectedEnd) {
        throw BmpError("file is truncated");
    }
    if (bfOffBits < kHeaderTotal) {
        throw BmpError("invalid bfOffBits");
    }

    in.seekg(static_cast<std::streamoff>(bfOffBits), std::ios::beg);
    if (!in) {
        throw BmpError("cannot seek to pixel data");
    }

    Image image(width, height);
    std::vector<unsigned char> row(rowSize);

    for (size_t storedY = 0; storedY < height; ++storedY) {
        if (!in.read(reinterpret_cast<char*>(row.data()),
                     static_cast<std::streamsize>(rowSize))) {
            throw BmpError("failed to read pixel row");
        }
        const size_t y = topDown ? storedY : (height - 1 - storedY);
        for (size_t x = 0; x < width; ++x) {
            const unsigned char b = row[x * 3 + 0];
            const unsigned char g = row[x * 3 + 1];
            const unsigned char r = row[x * 3 + 2];
            image.At(x, y) = {r / 255.0f, g / 255.0f, b / 255.0f};
        }
    }

    return image;
}

void WriteBmp(const std::string& path, const Image& image) {
    if (image.Empty()) {
        throw BmpError("refusing to write empty image");
    }
    const size_t width = image.Width();
    const size_t height = image.Height();
    const uint32_t rowSize = RowSize(width);
    const uint64_t pixelBytes = static_cast<uint64_t>(rowSize) * height;
    const uint64_t fileSize = kHeaderTotal + pixelBytes;
    if (fileSize > 0xFFFFFFFFull) {
        throw BmpError("image too large to store in BMP32");
    }

    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw BmpError("cannot open output file: " + path);
    }

    // BITMAPFILEHEADER
    WriteLE<uint16_t>(out, kSignature);
    WriteLE<uint32_t>(out, static_cast<uint32_t>(fileSize));
    WriteLE<uint16_t>(out, 0);
    WriteLE<uint16_t>(out, 0);
    WriteLE<uint32_t>(out, kHeaderTotal);

    // BITMAPINFOHEADER
    WriteLE<uint32_t>(out, kInfoHeaderSize);
    WriteLE<uint32_t>(out, static_cast<uint32_t>(width));
    WriteLE<uint32_t>(out, static_cast<uint32_t>(height));
    WriteLE<uint16_t>(out, 1);
    WriteLE<uint16_t>(out, kBitCount24);
    WriteLE<uint32_t>(out, kCompressionNone);
    WriteLE<uint32_t>(out, static_cast<uint32_t>(pixelBytes));
    WriteLE<uint32_t>(out, 2835);  // ~72 DPI
    WriteLE<uint32_t>(out, 2835);
    WriteLE<uint32_t>(out, 0);
    WriteLE<uint32_t>(out, 0);

    std::vector<unsigned char> row(rowSize, 0);
    for (size_t storedY = 0; storedY < height; ++storedY) {
        const size_t y = height - 1 - storedY;  // bottom-up
        for (size_t x = 0; x < width; ++x) {
            const Color c = image.At(x, y);
            row[x * 3 + 0] = ToByte(c.b);
            row[x * 3 + 1] = ToByte(c.g);
            row[x * 3 + 2] = ToByte(c.r);
        }
        out.write(reinterpret_cast<const char*>(row.data()),
                  static_cast<std::streamsize>(rowSize));
    }
    if (!out) {
        throw BmpError("write failed: " + path);
    }
}

}  // namespace ip