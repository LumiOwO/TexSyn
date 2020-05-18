#include "pyramid.h"
#include <opencv2/opencv.hpp>

namespace texture {

Pyramid::Pyramid(const cv::Mat& img, int k, int neighbor):
    _pyramid(k), _neighbor(neighbor)
{
    _pyramid[0] = img.clone();
    for (int i = 1; i < k; i++) {
        cv::pyrDown(_pyramid[i - 1], _pyramid[i]);
    }
}

void Pyramid::setColor(Color color, int row, int col, int k)
{
    cv::Mat &img = _pyramid[k];
    int index = (row * img.cols + col) * img.channels();
    uchar *data = img.ptr<uchar>(0) + index;
    data[0] = color[0];
    data[1] = color[1];
    data[2] = color[2];
}

std::vector<std::pair<int, int> > Pyramid::range(int row, int col, int k) const
{
    int len = 1 << k;
    std::vector<std::pair<int, int> > ret(len << k);
    std::pair<int, int> o{row << k, col << k};
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < len; j++) {
            ret[(i << k) + j] = {o.first + i, o.second + j};
        }
    }
    return ret;
}

TSVQ* Pyramid::tree(int k) const
{
    int rows = _pyramid[k].rows;
    int cols = _pyramid[k].cols;
    int size = rows * cols;
    std::vector<std::vector<uchar>> eigens(size);
    std::vector<Color> colors(size);

    int pos = 0;
    int index = 0;
    const uchar* data = _pyramid[k].ptr<uchar>(0);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            eigens[pos] = eigenAt(i, j, k);
            colors[pos] = { data[index], data[index + 1], data[index + 2]};
            pos++;
            index += 3;
        }
    }

    return new TSVQ(eigens, colors);
}

std::vector<uchar> Pyramid::eigenAt(int row, int col, int k) const
{
    std::vector<uchar> ret;
    // Current resolution: consider only left and above pixels
    int half = _neighbor >> 1;
    std::pair<int, int> nw{row - half, col - half};
    while (nw.first < 0) nw.first += _pyramid[k].rows;
    while (nw.second < 0) nw.second += _pyramid[k].cols;

    // Above
    int cols_3 = 3 * _pyramid[k].cols;
    for (int i = 0; i < half; i++) {
        const uchar* data = _pyramid[k].ptr<uchar>((nw.first + i) % _pyramid[k].rows);
        int index = nw.second * 3;
        for (int j = 0; j < _neighbor; j++) {
            ret.insert(ret.end(), { data[index], data[index + 1], data[index + 2] });
            index += 3;
            if (index > cols_3) index = 0;
        }
    }
    // Left
    {
        const uchar* data = _pyramid[k].ptr<uchar>((nw.first + half) % _pyramid[k].rows);
        int index = nw.second * 3;
        for (int j = 0; j < half; j++) {
            ret.insert(ret.end(), { data[index], data[index + 1], data[index + 2] });
            index += 3;
            if (index > cols_3) index = 0;
        }
    }

    // Lower resolutions
    // 6 -> 3, 5 -> 3, 4 -> 2, 3 -> 2
    int neighbor = _neighbor;
    for (int level = k + 1, n = _pyramid.size(); level < n; level++) {
        nw = {nw.first >> 1, nw.second >> 1};
        neighbor = (neighbor + 1) >> 1;
        cols_3 = 3 * _pyramid[level].cols;

        for (int i = 0; i < neighbor; i++) {
            const uchar* data = _pyramid[level].ptr<uchar>((nw.first + i) % _pyramid[level].rows);
            int index = nw.second * 3;
            for (int j = 0; j < neighbor; j++) {
                ret.insert(ret.end(), { data[index], data[index + 1], data[index + 2] });
                index += 3;
                if (index > cols_3) index = 0;
            }
        }
    }
    return ret;
}

};

