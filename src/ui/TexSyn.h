#pragma once

#include <QtWidgets/QMainWindow>
#include <ui_TexSyn.h>

#include <opencv2/opencv.hpp>
#include <QThread>
#include <vector>

namespace texture {
class Worker;
};

class TexSyn : public QMainWindow
{
    Q_OBJECT

public:
    TexSyn(QWidget *parent = Q_NULLPTR);
    ~TexSyn();

signals:
    void synthesize(const cv::Mat* pInput, int rows, int cols,
                    int levels, int neighbor_size);

public slots:
    void updateResult(const cv::Mat* res);
    void updateResultPixel(int row, int col, uchar r, uchar g, uchar b);
    void showResulotion(int k);
    void showRunningTime(double s);

private slots:
    void loadImg();
    void saveImg();
    void run();
    void stop();

private:
    Ui::TexSynClass ui;

    cv::Mat             _example;

    QImage              _example_qt;
    QImage              _result_qt;

    QThread             _workerThread;
    texture::Worker*    _worker;

private:
    void reset() {
        _result_qt = QImage();
        ui.result->clear();

        ui.kLevelEdit->setText("1");
        ui.neighborEdit->setText("5");
        ui.infoLabel->setText("");
    }

    static int clamp(QLineEdit *edit, int a, int b) {
        QString intstr = edit->text();
        if (intstr.isEmpty()) intstr = QString::number(a);
        int ret = std::min(std::max(intstr.toInt(), a), b);
        edit->setText(QString::number(ret));
        return ret;
    }

    static QImage cvMatToQImage(const cv::Mat& cvMat) {
        return QImage(
            (const unsigned char*)(cvMat.data),
            cvMat.cols, cvMat.rows,
            cvMat.cols * cvMat.channels(),
            QImage::Format_RGB888
        );
    }
};
