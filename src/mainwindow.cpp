#include "mainwindow.h"
#include "settings.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QSettings>
#include <QLabel>
#include <QGroupBox>
#include <iostream>

void MainWindow::initialize() {
    realtime = new Realtime;
    aspectRatioWidget = new AspectRatioWidget(this);
    aspectRatioWidget->setAspectWidget(realtime, 3.f/4.f);
    QHBoxLayout *hLayout = new QHBoxLayout; // horizontal alignment
    QVBoxLayout *vLayout = new QVBoxLayout(); // vertical alignment
    vLayout->setAlignment(Qt::AlignTop);
    hLayout->addLayout(vLayout);
    hLayout->addWidget(aspectRatioWidget, 1);
    this->setLayout(hLayout);

    // Create labels in sidebox
    QFont font;
    font.setPointSize(12);
    font.setBold(true);
    QLabel *tesselation_label = new QLabel(); // Parameters label
    tesselation_label->setText("Terrain Complexity");
    tesselation_label->setFont(font);
    QLabel *camera_label = new QLabel(); // Camera label
    camera_label->setText("Camera Settings");
    camera_label->setFont(font);

    QLabel *weather_label = new QLabel(); // Weather label
    weather_label->setText("Weather and time");
    weather_label->setFont(font);
    QLabel *intensity_label = new QLabel(); // Intensity label
    intensity_label->setText("Intensity:");
    QLabel *time_label = new QLabel(); // Time label
    time_label->setText("Change time:");

    QLabel *filters_label = new QLabel(); // Filters label
    filters_label->setText("Frame Filters");
    filters_label->setFont(font);
    QLabel *ec_label = new QLabel(); // Extra Credit label
    ec_label->setText("Rasterization Effects");
    ec_label->setFont(font);
    QLabel *param1_label = new QLabel(); // Parameter 1 label
    param1_label->setText("Parameter 1:");
    QLabel *param2_label = new QLabel(); // Parameter 2 label
    param2_label->setText("Parameter 2:");
    QLabel *near_label = new QLabel(); // Near plane label
    near_label->setText("Near Plane:");
    QLabel *far_label = new QLabel(); // Far plane label
    far_label->setText("Far Plane:");

    // Create checkbox for per-pixel filter
    filter1 = new QCheckBox();
    filter1->setText(QStringLiteral("Per-Pixel Filter"));
    filter1->setChecked(false);

    // Create checkbox for kernel-based filter
    filter2 = new QCheckBox();
    filter2->setText(QStringLiteral("Kernel-Based Filter"));
    filter2->setChecked(false);

    // Create file uploader for scene file
    uploadFile = new QPushButton();
    uploadFile->setText(QStringLiteral("Upload Scene File"));
    
    saveImage = new QPushButton();
    saveImage->setText(QStringLiteral("Save image"));

    // Creates the boxes containing the parameter sliders and number boxes
    QGroupBox *p1Layout = new QGroupBox(); // horizonal slider 1 alignment
    QHBoxLayout *l1 = new QHBoxLayout();
//    QGroupBox *p2Layout = new QGroupBox(); // horizonal slider 2 alignment
//    QHBoxLayout *l2 = new QHBoxLayout();

    // Create slider controls to control parameters
    p1Slider = new QSlider(Qt::Orientation::Horizontal); // Parameter 1 slider
    p1Slider->setTickInterval(1);
    p1Slider->setMinimum(1);
    p1Slider->setMaximum(25);
    p1Slider->setValue(1);

    p1Box = new QSpinBox();
    p1Box->setMinimum(1);
    p1Box->setMaximum(25);
    p1Box->setSingleStep(1);
    p1Box->setValue(1);

//    p2Slider = new QSlider(Qt::Orientation::Horizontal); // Parameter 2 slider
//    p2Slider->setTickInterval(1);
//    p2Slider->setMinimum(1);
//    p2Slider->setMaximum(25);
//    p2Slider->setValue(1);

//    p2Box = new QSpinBox();
//    p2Box->setMinimum(1);
//    p2Box->setMaximum(25);
//    p2Box->setSingleStep(1);
//    p2Box->setValue(1);

    // Adds the slider and number box to the parameter layouts
    l1->addWidget(p1Slider);
    l1->addWidget(p1Box);
    p1Layout->setLayout(l1);

//    l2->addWidget(p2Slider);
//    l2->addWidget(p2Box);
//    p2Layout->setLayout(l2);

    // Creates the boxes containing the camera sliders and number boxes
    QGroupBox *nearLayout = new QGroupBox(); // horizonal near slider alignment
    QHBoxLayout *lnear = new QHBoxLayout();
    QGroupBox *farLayout = new QGroupBox(); // horizonal far slider alignment
    QHBoxLayout *lfar = new QHBoxLayout();

    // Create slider controls to control near/far planes
    nearSlider = new QSlider(Qt::Orientation::Horizontal); // Near plane slider
    nearSlider->setTickInterval(1);
    nearSlider->setMinimum(1);
    nearSlider->setMaximum(1000);
    nearSlider->setValue(10);

    nearBox = new QDoubleSpinBox();
    nearBox->setMinimum(0.01f);
    nearBox->setMaximum(10.f);
    nearBox->setSingleStep(0.1f);
    nearBox->setValue(0.1f);

    farSlider = new QSlider(Qt::Orientation::Horizontal); // Far plane slider
    farSlider->setTickInterval(1);
    farSlider->setMinimum(1000);
    farSlider->setMaximum(10000);
    farSlider->setValue(10000);

    farBox = new QDoubleSpinBox();
    farBox->setMinimum(10.f);
    farBox->setMaximum(100.f);
    farBox->setSingleStep(0.1f);
    farBox->setValue(100.f);

    // Adds the slider and number box to the parameter layouts
    lnear->addWidget(nearSlider);
    lnear->addWidget(nearBox);
    nearLayout->setLayout(lnear);

    lfar->addWidget(farSlider);
    lfar->addWidget(farBox);
    farLayout->setLayout(lfar);

    // Weather:
    snow = new QCheckBox();
    snow->setText(QStringLiteral("Snow"));
    snow->setChecked(true);

    accumulate = new QCheckBox();
    accumulate->setText(QStringLiteral("Accumulate"));
    accumulate->setChecked(false);

    sun = new QCheckBox();
    sun->setText(QStringLiteral("Sun"));
    sun->setChecked(false);

    QGroupBox *intensityLayout = new QGroupBox(); // horizonal slider alignment
    QHBoxLayout *intensity = new QHBoxLayout();

    intensitySlider = new QSlider(Qt::Orientation::Horizontal); // Intensity slider
    intensitySlider->setTickInterval(1);
    intensitySlider->setMinimum(50);
    intensitySlider->setMaximum(200);
    intensitySlider->setValue(50);

    intensityBox = new QSpinBox();
    intensityBox->setMinimum(50);
    intensityBox->setMaximum(200);
    intensityBox->setSingleStep(1);
    intensityBox->setValue(50);

    intensity->addWidget(intensitySlider);
    intensity->addWidget(intensityBox);
    intensityLayout->setLayout(intensity);

    QGroupBox *timeLayout = new QGroupBox(); // horizonal slider alignment
    QHBoxLayout *time = new QHBoxLayout();

    timeSlider = new QSlider(Qt::Orientation::Horizontal); // Time slider
    timeSlider->setTickInterval(1);
    timeSlider->setMinimum(0);
    timeSlider->setMaximum(23);
    timeSlider->setValue(6);

    timeBox = new QSpinBox();
    timeBox->setMinimum(0);
    timeBox->setMaximum(23);
    timeBox->setSingleStep(1);
    timeBox->setValue(6);

    time->addWidget(timeSlider);
    time->addWidget(timeBox);
    timeLayout->setLayout(time);

    // Extra Credit:
    ec1 = new QCheckBox();
    ec1->setText(QStringLiteral("Adaptive Number of Objects"));
    ec1->setChecked(false);

    ec2 = new QCheckBox();
    ec2->setText(QStringLiteral("Adaptive Distance from Camera"));
    ec2->setChecked(false);

    ec3 = new QCheckBox();
    ec3->setText(QStringLiteral("FXAA"));
    ec3->setChecked(false);

    ec4 = new QCheckBox();
    ec4->setText(QStringLiteral("Extra Credit 4"));
    ec4->setChecked(false);

    vLayout->addWidget(uploadFile);
    vLayout->addWidget(saveImage);

    //Weather
    vLayout->addWidget(weather_label);
    vLayout->addWidget(snow);
    vLayout->addWidget(accumulate);
    vLayout->addWidget(intensity_label);
    vLayout->addWidget(intensityLayout);
    vLayout->addWidget(sun);
    vLayout->addWidget(time_label);
    vLayout->addWidget(timeLayout);

    vLayout->addWidget(tesselation_label);
    vLayout->addWidget(param1_label);
    vLayout->addWidget(p1Layout);
//    vLayout->addWidget(param2_label);
//    vLayout->addWidget(p2Layout);
    vLayout->addWidget(camera_label);
    vLayout->addWidget(near_label);
    vLayout->addWidget(nearLayout);
    vLayout->addWidget(far_label);
    vLayout->addWidget(farLayout);
    vLayout->addWidget(filters_label);
    vLayout->addWidget(filter1);
    vLayout->addWidget(filter2);
    // Extra Credit:
    vLayout->addWidget(ec_label);
    vLayout->addWidget(ec1);
    vLayout->addWidget(ec2);
    vLayout->addWidget(ec3);
//    vLayout->addWidget(ec4);

    connectUIElements();

    // Set default values of 5 for tesselation parameters
    onValChangeP1(5);
//    onValChangeP2(5);

    onValChangeIntensity(50);

    // Set default values for near and far planes
    onValChangeNearBox(0.1f);
    onValChangeFarBox(10.f);
}

void MainWindow::finish() {
    realtime->finish();
    delete(realtime);
}

// ********************************* Connection functions *********************************
void MainWindow::connectUIElements() {
    connectPerPixelFilter();
    connectKernelBasedFilter();
    connectUploadFile();
    connectSaveImage();
    connectParam1();
//    connectParam2();
    connectNear();
    connectFar();
    connectExtraCredit();

    connectSnow();
    connectAccumulate();
    connectIntensity();
}

void MainWindow::connectPerPixelFilter() {
    connect(filter1, &QCheckBox::clicked, this, &MainWindow::onPerPixelFilter);
}

void MainWindow::connectKernelBasedFilter() {
    connect(filter2, &QCheckBox::clicked, this, &MainWindow::onKernelBasedFilter);
}

void MainWindow::connectUploadFile() {
    connect(uploadFile, &QPushButton::clicked, this, &MainWindow::onUploadFile);
}

void MainWindow::connectSaveImage() {
    connect(saveImage, &QPushButton::clicked, this, &MainWindow::onSaveImage);
}

void MainWindow::connectParam1() {
    connect(p1Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeP1);
    connect(p1Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeP1);
}

//void MainWindow::connectParam2() {
//    connect(p2Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeP2);
//    connect(p2Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
//            this, &MainWindow::onValChangeP2);
//}

void MainWindow::connectNear() {
    connect(nearSlider, &QSlider::valueChanged, this, &MainWindow::onValChangeNearSlider);
    connect(nearBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onValChangeNearBox);
}

void MainWindow::connectFar() {
    connect(farSlider, &QSlider::valueChanged, this, &MainWindow::onValChangeFarSlider);
    connect(farBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onValChangeFarBox);
}

void MainWindow::connectExtraCredit() {
    connect(ec1, &QCheckBox::clicked, this, &MainWindow::onExtraCredit1);
    connect(ec2, &QCheckBox::clicked, this, &MainWindow::onExtraCredit2);
    connect(ec3, &QCheckBox::clicked, this, &MainWindow::onExtraCredit3);
    connect(ec4, &QCheckBox::clicked, this, &MainWindow::onExtraCredit4);
}

// Weather:

void MainWindow::connectSnow() {
    connect(snow, &QCheckBox::clicked, this, &MainWindow::onSnow);
}

void MainWindow::connectAccumulate() {
    connect(accumulate, &QCheckBox::clicked, this, &MainWindow::onAccumulate);
}

void MainWindow::connectSun() {
    connect(accumulate, &QCheckBox::clicked, this, &MainWindow::onSun);
}

void MainWindow::connectIntensity() {
    connect(intensitySlider, &QSlider::valueChanged, this, &MainWindow::onValChangeIntensity);
    connect(intensityBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeIntensity);
}

void MainWindow::connectTime() {
    connect(timeSlider, &QSlider::valueChanged, this, &MainWindow::onValChangeTime);
    connect(timeBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeTime);
}

// ********************************* Slots *********************************
void MainWindow::onPerPixelFilter() {
    settings.perPixelFilter = !settings.perPixelFilter;
    realtime->settingsChanged();
}

void MainWindow::onKernelBasedFilter() {
    settings.kernelBasedFilter = !settings.kernelBasedFilter;
    realtime->settingsChanged();
}

void MainWindow::onUploadFile() {
    // Get abs path of scene file
    QString configFilePath = QFileDialog::getOpenFileName(this, tr("Upload File"),
                                                          QDir::currentPath()
                                                              .append(QDir::separator())
                                                              .append("scenefiles")
                                                              .append(QDir::separator())
                                                              .append("lights-camera")
                                                              .append(QDir::separator())
                                                              .append("required"), tr("Scene Files (*.json)"));
    if (configFilePath.isNull()) {
        std::cout << "Failed to load null scenefile." << std::endl;
        return;
    }

    settings.sceneFilePath = configFilePath.toStdString();

    std::cout << "Loaded scenefile: \"" << configFilePath.toStdString() << "\"." << std::endl;

    realtime->sceneChanged();
}

void MainWindow::onSaveImage() {
    if (settings.sceneFilePath.empty()) {
        std::cout << "No scene file loaded." << std::endl;
        return;
    }
    std::string sceneName = settings.sceneFilePath.substr(0, settings.sceneFilePath.find_last_of("."));
    sceneName = sceneName.substr(sceneName.find_last_of("/")+1);
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Image"),
                                                    QDir::currentPath()
                                                        .append(QDir::separator())
                                                        .append("student_outputs")
                                                        .append(QDir::separator())
                                                        .append("lights-camera")
                                                        .append(QDir::separator())
                                                        .append("required")
                                                        .append(QDir::separator())
                                                        .append(sceneName), tr("Image Files (*.png)"));
    std::cout << "Saving image to: \"" << filePath.toStdString() << "\"." << std::endl;
    realtime->saveViewportImage(filePath.toStdString());
}

void MainWindow::onValChangeP1(int newValue) {
    p1Slider->setValue(newValue);
    p1Box->setValue(newValue);
    settings.shapeParameter1 = p1Slider->value();
    realtime->settingsChanged();
}

//void MainWindow::onValChangeP2(int newValue) {
//    p2Slider->setValue(newValue);
//    p2Box->setValue(newValue);
//    settings.shapeParameter2 = p2Slider->value();
//    realtime->settingsChanged();
//}

void MainWindow::onValChangeNearSlider(int newValue) {
    //nearSlider->setValue(newValue);
    nearBox->setValue(newValue/100.f);
    settings.nearPlane = nearBox->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeFarSlider(int newValue) {
    //farSlider->setValue(newValue);
    farBox->setValue(newValue/100.f);
    settings.farPlane = farBox->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeNearBox(double newValue) {
    nearSlider->setValue(int(newValue*100.f));
    //nearBox->setValue(newValue);
    settings.nearPlane = nearBox->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeFarBox(double newValue) {
    farSlider->setValue(int(newValue*100.f));
    //farBox->setValue(newValue);
    settings.farPlane = farBox->value();
    realtime->settingsChanged();
}

// Weather:

void MainWindow::onSnow() {
    settings.snow = !settings.snow;
    realtime->settingsChanged();
}

void MainWindow::onAccumulate() {
    settings.accumulate = !settings.accumulate;
    realtime->settingsChanged();
}

void MainWindow::onSun() {
    settings.sun = !settings.sun;
    realtime->settingsChanged();
}

void MainWindow::onValChangeIntensity(int newValue) {
    intensitySlider->setValue(newValue);
    intensityBox->setValue(newValue);
    settings.intensity = intensitySlider->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeTime(int newValue) {
    timeSlider->setValue(newValue);
    timeBox->setValue(newValue);
    settings.time = timeSlider->value();
    realtime->settingsChanged();
}

// Extra Credit:

void MainWindow::onExtraCredit1() {
    settings.extraCredit1 = !settings.extraCredit1;
    realtime->settingsChanged();
}

void MainWindow::onExtraCredit2() {
    settings.extraCredit2 = !settings.extraCredit2;
    realtime->settingsChanged();
}

void MainWindow::onExtraCredit3() {
    settings.extraCredit3 = !settings.extraCredit3;
    realtime->settingsChanged();
}

void MainWindow::onExtraCredit4() {
    settings.extraCredit4 = !settings.extraCredit4;
    realtime->settingsChanged();
}
