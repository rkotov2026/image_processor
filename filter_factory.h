#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "filters.h"

namespace ip {

// Сколько параметров ожидает фильтр с данным именем.
// -1 — фильтр не найден.
int ExpectedParamCount(std::string_view name);

// Создаёт фильтр по имени и строковым параметрам.
// Бросает ArgsError, если имя неизвестно или параметры некорректны.
std::unique_ptr<Filter> CreateFilter(std::string_view name,
                                     const std::vector<std::string>& params);

}  // namespace ip