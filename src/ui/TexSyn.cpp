#include "TexSyn.h"

#include <QIntValidator>
#include <QFileDialog>
#include <QMessageBox>

#include <texture/texture.h>

TexSyn::TexSyn(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    QIntValidator *validator = new QIntValidator(0, 999, this);
    ui.widthEdit->setValidator(validator);
    ui.heightEdit->setValidator(validator);

    connect(ui.loadButton, SIGNAL(clicked(bool)), this, SLOT(loadImg()));
    connect(ui.saveButton, SIGNAL(clicked(bool)), this, SLOT(saveImg()));
    connect(ui.runButton, SIGNAL(clicked(bool)), this, SLOT(run()));

    connect(ui.widthEdit, SIGNAL(returnPressed()), this, SLOT(run()));
    connect(ui.heightEdit, SIGNAL(returnPressed()), this, SLOT(run()));

}

void TexSyn::loadImg()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Load example texture",
        "./examples",
        "Image Files(*.jpg *.png *.bmp *.pgm *.pbm);;All(*.*)");
    if (filename.isEmpty()) return;

    _example = cv::imread(filename.toStdString());
    if (_example.empty()) {
        QMessageBox::critical(NULL, "Error", "Fail to load example texture.",
            QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    if (_example.channels() == 3) {
        // convert Mat BGR to QImage RGB
        cv::cvtColor(_example, _example, cv::COLOR_BGR2RGB);
    }
    _example_qt = cvMatToQImage(_example);

    ui.example->setPixmap(QPixmap::fromImage(_example_qt));
    ui.widthEdit->setText(QString::number(_example.cols));
    ui.heightEdit->setText(QString::number(_example.rows));

    _result = cv::Mat();
    _result_qt = QImage();
    ui.result->clear();
}

void TexSyn::saveImg()
{
    if (_result_qt.isNull()) return;

    QString filename = QFileDialog::getSaveFileName(
        this,
        "Save current texture",
        "./examples",
        "Images (*.jpg)");
    if (filename.isEmpty()) return;

    if (!_result_qt.save(filename)) {
        QMessageBox::critical(NULL, "Error", "Fail to save current texture.",
            QMessageBox::Ok, QMessageBox::Ok);
    }
}

void TexSyn::run()
{
    if (_example_qt.isNull()) return;

    int width = clamp(ui.widthEdit->text());
    int height = clamp(ui.heightEdit->text());
    ui.widthEdit->setText(QString::number(width));
    ui.heightEdit->setText(QString::number(height));

    // Core function
    _result = texture::synthesize(_example, height, width);

    _result_qt = cvMatToQImage(_result);
    ui.result->setPixmap(QPixmap::fromImage(_result_qt));
}

int TexSyn::clamp(QString intstr) const
{
    const int minPixel = _example.cols;
    const int maxPixel = 650;

    if (intstr.isEmpty()) intstr = QString::number(minPixel);
    int ret = intstr.toInt();
    return std::min(std::max(ret, minPixel), maxPixel);
}

QImage TexSyn::cvMatToQImage(const cv::Mat& cvMat) const
{
    return QImage(
        (const unsigned char*)(cvMat.data),
        cvMat.cols, cvMat.rows,
        cvMat.cols * cvMat.channels(),
        QImage::Format_RGB888
    );
}
