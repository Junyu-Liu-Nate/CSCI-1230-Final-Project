#pragma once

#include <QMainWindow>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include "realtime.h"
#include "utils/aspectratiowidget/aspectratiowidget.hpp"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    void initialize();
    void finish();

private:
    void connectUIElements();
    void connectSpeed();
    void connectParam1();
//    void connectParam2();
    void connectNear();
    void connectFar();
    void connectPerPixelFilter();
    void connectKernelBasedFilter();
    void connectUploadFile();
    void connectUploadHeightMap();
    void connectSaveImage();
    void connectExtraCredit();

    //Weather
    void connectSnow();
    void connectAccumulate();
    void connectSun();
    void connectIntensity();
    void connectTime();

    Realtime *realtime;
    AspectRatioWidget *aspectRatioWidget;
    QCheckBox *filter1;
    QCheckBox *filter2;
    QPushButton *uploadFile;
    QPushButton *uploadHeightMap;
    QPushButton *saveImage;
    QSlider *speedSlider;
    QSpinBox *speedBox;
    QSlider *bumpinessSlider;
    QSpinBox *bumpinessBox;
    QSlider *nearSlider;
    QSlider *farSlider;
    QDoubleSpinBox *nearBox;
    QDoubleSpinBox *farBox;

    //Weather
    QCheckBox *snow;
    QCheckBox *accumulate;
    QCheckBox *sun;
    QSlider *intensitySlider;
    QSpinBox *intensityBox;
    QSlider *timeSlider;
    QSpinBox *timeBox;

    // Extra Credit:
    QCheckBox *ec1;
    QCheckBox *ec2;
    QCheckBox *ec3;
    QCheckBox *ec4;

private slots:
    void onPerPixelFilter();
    void onKernelBasedFilter();
    void onUploadFile();
    void onUploadHeightMap();
    void onSaveImage();
    void onValChangeSpeed(int newValue);
    void onValChangeBumpiness(int newValue);
    void onValChangeNearSlider(int newValue);
    void onValChangeFarSlider(int newValue);
    void onValChangeNearBox(double newValue);
    void onValChangeFarBox(double newValue);

    //Weather
    void onSnow();
    void onAccumulate();
    void onSun();
    void onValChangeIntensity(int newValue);
    void onValChangeTime(int newValue);

    // Extra Credit:
    void onExtraCredit1();
    void onExtraCredit2();
    void onExtraCredit3();
    void onExtraCredit4();
};
