#include "TexSyn.h"

#include <QIntValidator>
#include <QFileDialog>
#include <QMessageBox>

#include <texture/texture.h>
#include <chrono>

TexSyn::TexSyn(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    QIntValidator *validator = new QIntValidator(0, 999, this);
    ui.widthEdit->setValidator(validator);
    ui.heightEdit->setValidator(validator);
    ui.kLevelEdit->setValidator(validator);
    ui.neighborEdit->setValidator(validator);

    connect(ui.loadButton, SIGNAL(clicked(bool)), this, SLOT(loadImg()));
    connect(ui.saveButton, SIGNAL(clicked(bool)), this, SLOT(saveImg()));
    connect(ui.runButton,  SIGNAL(clicked(bool)), this, SLOT(run()));
    connect(ui.stopButton, SIGNAL(clicked(bool)), this, SLOT(stop()));

    reset();

    // Manage a thread for synthesize
    using texture::Worker;
    _worker = new Worker();
    _worker->moveToThread(&_workerThread);
    // Synthesize start
    connect(this, &TexSyn::synthesize, _worker, &Worker::synthesize);
    // Result callback
    connect(_worker, &Worker::updateResult,      this, &TexSyn::updateResult);
    connect(_worker, &Worker::updateResultPixel, this, &TexSyn::updateResultPixel);
    connect(_worker, &Worker::showResulotion,    this, &TexSyn::showResulotion);
    connect(_worker, &Worker::showRunningTime,   this, &TexSyn::showRunningTime);

    _workerThread.start();
}

TexSyn::~TexSyn()
{
    _workerThread.quit();
    _workerThread.wait();

    delete _worker;
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

    reset();
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

    int width = clamp(ui.widthEdit, _example.cols, ui.result->width());
    int height = clamp(ui.heightEdit, _example.cols, ui.result->width());
    int kLevel = clamp(ui.kLevelEdit, 1, 5);
    int neighbor = clamp(ui.neighborEdit, 3, 13);
    
    ui.result->clear();
    _result_qt = QImage(width, height, QImage::Format_RGB888);

    // Core function, running in a worker thread
    emit synthesize(&_example, height, width, kLevel, neighbor);
}

void TexSyn::stop()
{
    // Restart worker thread
    _workerThread.terminate();
    _workerThread.wait();

    _workerThread.start();

    ui.infoLabel->setText("Interrupted.");
    ui.result->setPixmap(QPixmap::fromImage(_result_qt));
}

void TexSyn::updateResult(const cv::Mat* res)
{
    _result_qt = cvMatToQImage(*res);
    ui.result->setPixmap(QPixmap::fromImage(_result_qt));
    ui.result->repaint();
}

void TexSyn::updateResultPixel(int row, int col, uchar r, uchar g, uchar b)
{
    QRgb rgb = 0xFF000000 + (QRgb(r) << 16) + (QRgb(g) << 8) + (QRgb(b) << 0);
    _result_qt.setPixel({ col, row }, rgb);

    if (col == _result_qt.width() - 1
        && (row == _result_qt.height() - 1 
            || (row > 0 && (row % (_result_qt.height() >> 3) == 0) ) )) {

        ui.result->setPixmap(QPixmap::fromImage(_result_qt));
        ui.result->repaint();
    }
}

void TexSyn::showResulotion(int k)
{
    int w = _result_qt.width() >> k;
    int h = _result_qt.height() >> k;
    ui.infoLabel->setText(
        QString("Processing resolution ") 
        + QString::number(w) + " x " + QString::number(h) + "..."
    );
    ui.infoLabel->repaint();
}

void TexSyn::showRunningTime(double s)
{
    ui.infoLabel->setText(
        QString("Finished in ") + QString::number(s) + "s."
    );
}

