#include "bmp.h"
#include "exceptions.h"
#include "filter_factory.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace {

struct FilterSpec {
    std::string name;
    std::vector<std::string> params;
};

struct Args {
    std::string input;
    std::string output;
    std::vector<FilterSpec> filters;
};

Args ParseArgs(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        throw ip::ArgsError("expected input and output file paths");
    }

    Args result;
    result.input = args[0];
    result.output = args[1];

    size_t i = 2;
    while (i < args.size()) {
        const std::string& token = args[i];
        if (token.size() < 2 || token[0] != '-') {
            throw ip::ArgsError("expected filter name starting with '-', got: '" +
                                token + "'");
        }
        FilterSpec spec;
        spec.name = token.substr(1);

        const int expected = ip::ExpectedParamCount(spec.name);
        if (expected < 0) {
            throw ip::ArgsError("unknown filter: '" + spec.name + "'");
        }
        if (i + static_cast<size_t>(expected) >= args.size()) {
            throw ip::ArgsError("not enough parameters for filter '" +
                                spec.name + "'");
        }
        for (int k = 1; k <= expected; ++k) {
            spec.params.push_back(args[i + static_cast<size_t>(k)]);
        }
        i += static_cast<size_t>(expected) + 1;
        result.filters.push_back(std::move(spec));
    }

    return result;
}

void PrintHelp(const char* program) {
    std::cout
        << "Usage:\n"
        << "  " << program
        << " <input.bmp> <output.bmp> [filters...]\n\n"
        << "Filters:\n"
        << "  -crop <width> <height>   crop to top-left rectangle\n"
        << "  -gs                      convert to grayscale\n"
        << "  -neg                     invert colors\n"
        << "  -sharp                   sharpen\n"
        << "  -edge <threshold>        edge detection (threshold in [0,1])\n"
        << "  -blur <sigma>            gaussian blur (sigma > 0)\n"
        << "  -pnt <radius> <seed>       pnt (radius > 0) seed (0 = random)\n\n"
        << "Example:\n"
        << "  " << program
        << " input.bmp output.bmp -crop 800 600 -gs -blur 0.5\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        PrintHelp(argv[0]);
        return 0;
    }

    try {
        std::vector<std::string> raw(argv + 1, argv + argc);
        const Args args = ParseArgs(raw);

        ip::Image image = ip::ReadBmp(args.input);

        for (const auto& spec : args.filters) {
            auto filter = ip::CreateFilter(spec.name, spec.params);
            image = filter->Apply(image);
        }

        ip::WriteBmp(args.output, image);
    } catch (const ip::ImageProcessorError& e) {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "unexpected error: " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "unexpected unknown error\n";
        return 1;
    }

    return 0;
}