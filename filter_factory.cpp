#include "filter_factory.h"

#include <stdexcept>
#include <string>
#include <utility>

#include "exceptions.h"

namespace ip {

namespace {

// ---- Внутренние хелперы: видны только в этом .cpp ----

int ParseInt(const std::string& s, const std::string& what) {
    size_t pos = 0;
    int value = 0;
    try {
        value = std::stoi(s, &pos);
    } catch (const std::exception&) {
        throw ArgsError("invalid integer for " + what + ": '" + s + "'");
    }
    if (pos != s.size()) {
        throw ArgsError("invalid integer for " + what + ": '" + s + "'");
    }
    return value;
}

double ParseDouble(const std::string& s, const std::string& what) {
    size_t pos = 0;
    double value = 0;
    try {
        value = std::stod(s, &pos);
    } catch (const std::exception&) {
        throw ArgsError("invalid number for " + what + ": '" + s + "'");
    }
    if (pos != s.size()) {
        throw ArgsError("invalid number for " + what + ": '" + s + "'");
    }
    return value;
}

}  // namespace

// ---- Публичные функции: видны снаружи ----

int ExpectedParamCount(std::string_view name) {
    if (name == CropFilter::Name)          return CropFilter::ParamCount;
    if (name == GrayscaleFilter::Name)     return GrayscaleFilter::ParamCount;
    if (name == NegativeFilter::Name)      return NegativeFilter::ParamCount;
    if (name == SharpeningFilter::Name)    return SharpeningFilter::ParamCount;
    if (name == EdgeDetectionFilter::Name) return EdgeDetectionFilter::ParamCount;
    if (name == GaussianBlurFilter::Name)  return GaussianBlurFilter::ParamCount;
    if (name == PointillizeFilter::Name)       return PointillizeFilter::ParamCount;
    return -1;
}

std::unique_ptr<Filter> CreateFilter(std::string_view name,
                                     const std::vector<std::string>& params) {
    if (name == CropFilter::Name) {
        const int w = ParseInt(params[0], "crop width");
        const int h = ParseInt(params[1], "crop height");
        if (w <= 0 || h <= 0) {
            throw ArgsError("crop: width and height must be positive");
        }
        return std::make_unique<CropFilter>(
            static_cast<size_t>(w), static_cast<size_t>(h));
    }
    if (name == GrayscaleFilter::Name) {
        return std::make_unique<GrayscaleFilter>();
    }
    if (name == NegativeFilter::Name) {
        return std::make_unique<NegativeFilter>();
    }
    if (name == SharpeningFilter::Name) {
        return std::make_unique<SharpeningFilter>();
    }
    if (name == EdgeDetectionFilter::Name) {
        const double t = ParseDouble(params[0], "edge threshold");
        return std::make_unique<EdgeDetectionFilter>(static_cast<float>(t));
    }
    if (name == GaussianBlurFilter::Name) {
        const double sigma = ParseDouble(params[0], "blur sigma");
        return std::make_unique<GaussianBlurFilter>(
            static_cast<float>(sigma));
    }
    if (name == PointillizeFilter::Name) {
        const int rad = ParseDouble(params[0], "pointillize radius");
        const int seed = ParseInt(params[1], "pointillize seed");
        return std::make_unique<PointillizeFilter>(static_cast<float>(rad), seed);
    }
    throw ArgsError("unknown filter: '" + std::string(name) + "'");
}

}  // namespace ip