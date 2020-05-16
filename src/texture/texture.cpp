#include "texture.h"

cv::Mat texture::synthesize(const cv::Mat& example, int rows, int cols)
{
    cv::Mat res = cv::Mat::zeros(rows, cols, example.type());
    return res;
}
