#pragma once

#include <opencv2/opencv.hpp>

namespace texture {

cv::Mat synthesize(const cv::Mat& example, int rows, int cols);

};