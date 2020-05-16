#pragma once

#include <QtWidgets/QMainWindow>
#include <ui_TexSyn.h>

#include <opencv2/opencv.hpp>

class TexSyn : public QMainWindow
{
    Q_OBJECT

public:
    TexSyn(QWidget *parent = Q_NULLPTR);

private slots:
    void loadImg();
    void saveImg();
    void run();

private:
    Ui::TexSynClass ui;

    cv::Mat     _example;
    cv::Mat     _result;

    QImage      _example_qt;
    QImage      _result_qt;
private:
    int clamp(QString intstr) const;
    QImage cvMatToQImage(const cv::Mat& cvMat) const;
};
