#pragma once

#include <stdexcept>
#include <string>

namespace ip {

// Базовое исключение проекта. Все остальные наследуются от него —
// так в main можно ловить одним catch'ем именно «свои» ошибки
class ImageProcessorError : public std::runtime_error {
public:
    explicit ImageProcessorError(const std::string& what)
        : std::runtime_error(what) {}
};

// Ошибки чтения/записи BMP
class BmpError : public ImageProcessorError {
public:
    explicit BmpError(const std::string& what)
        : ImageProcessorError("BMP: " + what) {}
};

// Ошибки разбора аргументов командной строки
class ArgsError : public ImageProcessorError {
public:
    explicit ArgsError(const std::string& what)
        : ImageProcessorError("args: " + what) {}
};

// Ошибки применения фильтров (неверные параметры, несовместимые размеры)
class FilterError : public ImageProcessorError {
public:
    explicit FilterError(const std::string& what)
        : ImageProcessorError("filter: " + what) {}
};

}  // namespace ip