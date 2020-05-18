#pragma once

#include <algorithm>
#include <vector>
#include <QObject>

namespace cv {
class Mat;
};
class TexSyn;

namespace texture {

inline int clamp(int num, int a, int b) {
    return std::min(std::max(num, a), b);
}

cv::Mat initialize(int rows, int cols, const cv::Mat& input);

void matchHistogram(cv::Mat& output, const cv::Mat& input);
std::vector<double> makeCDF(const cv::Mat& img);

// Worker for synthesize in another thread
class Worker : public QObject
{
    Q_OBJECT

public slots:
    void synthesize(const cv::Mat *pInput, int rows, int cols,
                    int levels, int neighbor_size);

signals:
    void updateResult(const cv::Mat* res);
    void updateResultPixel(int row, int col, uchar r, uchar g, uchar b);
    void showResulotion(int k);
    void showRunningTime(double s);
};

};