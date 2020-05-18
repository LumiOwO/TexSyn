#include "texture.h"
#include <texture/pyramid.h>

#include <ui/TexSyn.h>

#include <map>
#include <chrono>

namespace texture {

void Worker::synthesize(const cv::Mat *pInput, int rows, int cols,
                        int levels, int neighbor)
{
    const cv::Mat &input = *pInput;
    // Record running time
    using std::chrono::system_clock;
    using std::chrono::microseconds;
    // Start to record time
    auto startTime = system_clock::now();
    {
        // Initialize
        cv::Mat output = initialize(rows, cols, input);
        emit updateResult(&output);

        // Build pyramid
        Pyramid pyramid_in(input, levels, neighbor);
        Pyramid pyramid_out(output, levels, neighbor);

        // Loop for each level
        while (levels--) {
            emit showResulotion(levels);

            // Accelerate -- Build TSVQ struct for this level
            TSVQ *tree = pyramid_in.tree(levels);
            debug_print("Built TSVQ at level " << levels);

            // Loop for each pixel in this level
            auto size = pyramid_out.size(levels);
            for (int row = 0; row < size.first; row++) {
                for (int col = 0; col < size.second; col++) {
                    // Search best pixel color
                    auto eigen = pyramid_out.eigenAt(row, col, levels);
                    auto color = tree->bestMatch(eigen);
                    // Set output pixel
                    pyramid_out.setColor(color, row, col, levels);
                    // Set UI pixel 
                    for (auto p : pyramid_out.range(row, col, levels)) {
                        emit updateResultPixel(
                            p.first, p.second, color[0], color[1], color[2]);
                    }
                }
            }

            delete tree;
        }
    }
    // End to record time
    auto endTime = system_clock::now();
    auto duration = std::chrono::duration_cast<microseconds>(endTime - startTime);
    double s = double(duration.count()) * microseconds::period::num / microseconds::period::den;
    emit showRunningTime(s);
    
}

cv::Mat initialize(int rows, int cols, const cv::Mat& input) 
{
    cv::Mat output = cv::Mat::zeros(rows, cols, input.type());

    // Gaussian white noise
    cv::RNG rng;
    uchar* data = output.ptr<uchar>(0);
    for (int i = 0, n = rows * cols * output.channels(); i < n; i++) {
        int color = 127 + rng.gaussian(1.2) * 32;
        data[i] = clamp(color, 0, 255);
    }
    matchHistogram(output, input);

    return output;
}

void matchHistogram(cv::Mat& output, const cv::Mat& input)
{
    std::vector<double> cdf_in = makeCDF(input);
    std::vector<double> cdf_out = makeCDF(output);
    
    std::map<double, int> inv_cdf_in;
    for (int i = 0, n = cdf_in.size(); i < n; i++) {
        inv_cdf_in.insert({cdf_in[i], i});
    }
    inv_cdf_in[0.0] = 0;
    inv_cdf_in[1.0] = 255;

    uchar* data = output.ptr<uchar>(0);
    for (int i = 0, n = output.rows * output.cols * output.channels(); i < n; i++) {
        int index = data[i];
        double value = cdf_out[index];
        auto it = inv_cdf_in.lower_bound(value);

        int color = it->second;
        if (value != it->first) {
            while (it->first > value) it--;
            color = it->second + (color - it->second) / 2;
        }
        data[i] = color;
    }
}

std::vector<double> makeCDF(const cv::Mat& img)
{
    const int BINS = 256;

    std::vector<double> cdf(BINS, 0);
    const uchar* data = img.ptr<uchar>(0);
    int size = img.rows * img.cols;
    for (int i = 0, n = size * img.channels(); i < n; i += 3) {
        int index = (int)data[i] + (int)data[i + 1] + (int)data[i + 2];
        cdf[index / 3] += 1;
    }

    double inv_size = 1.0 / size;
    double sum = 0;
    for (int i = 0; i < BINS; i++) {
        sum += cdf[i] * inv_size;
        if (sum < 1.0) {
            cdf[i] = sum;
        } else for (; i < BINS; i++) {
            cdf[i] = 1.0;
        }
    }

    return cdf;
}

};
