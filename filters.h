#pragma once

#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "bmp.h"
#include "exceptions.h"
#include "filters/base_filter.h"
#include "filters/box_blur_filter.h"
#include "filters/crop_filter.h"
#include "filters/edge_detection_filter.h"
#include "filters/gaussian_blur_filter.h"
#include "filters/grayscale_filter.h"
#include "filters/negative_filter.h"
#include "filters/sharpening_filter.h"