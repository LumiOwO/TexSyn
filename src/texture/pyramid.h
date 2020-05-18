#pragma once

#include <opencv2/opencv.hpp>

#include <texture/TSVQ.h>

namespace texture {

class Pyramid
{
private:
    std::vector<cv::Mat> _pyramid;  // size big --> small
    int _neighbor;

public:
    Pyramid(const cv::Mat& img, int k, int neighbor);
    
    std::pair<int, int> size(int k) const {
        return { _pyramid[k].rows, _pyramid[k].cols };
    }

    void setColor(Color color, int row, int col, int k);

    std::vector<std::pair<int, int> > range(int row, int col, int k) const;

    TSVQ* tree(int k) const;

    std::vector<uchar> eigenAt(int row, int col, int k) const;

};

};


