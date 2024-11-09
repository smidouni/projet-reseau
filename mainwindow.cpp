#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QQmlContext>
#include <QUrl>
#include <QPushButton>
#include <QSlider>
#include <QRandomGenerator>

MainWindow::MainWindow(Graph *graph, double centerLat, double centerLon, int zoomLevel, QWidget *parent)
    : QMainWindow(parent), simManager(new SimulationManager(*graph)), centerLat(centerLat), centerLon(centerLon), zoomLevel(zoomLevel) {
    setupUI();
    setupMap();
    setupControls();

    // Generate a limited number of vehicles (e.g., 3) for performance testing
    for (int i = 0; i < 3; ++i) {
        qint64 startNodeId = graph->nodes.keys().at(QRandomGenerator::global()->bounded(graph->nodes.size()));
        simManager->addVehicle(i, startNodeId);
    }

    connect(simManager, &SimulationManager::updated, this, &MainWindow::updateMap);
}

void MainWindow::setupUI() {
    // Set up the main layout and central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Create the map view as a QQuickWidget
    mapView = new QQuickWidget;
    mapView->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mapView->setMinimumSize(800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(mapView);
    centralWidget->setLayout(mainLayout);
}

void MainWindow::setupMap() {
    // Set up QML map view and provide simManager to QML
    QQmlContext *context = mapView->rootContext();
    context->setContextProperty("simManager", simManager);

    // Pass center and zoom level as context properties
    context->setContextProperty("initialCenterLat", centerLat);
    context->setContextProperty("initialCenterLon", centerLon);
    context->setContextProperty("initialZoomLevel", zoomLevel);

    mapView->setSource(QUrl(QStringLiteral("qrc:/maCarte/MapView.qml")));
}

void MainWindow::setupControls() {
    QPushButton *pauseButton = new QPushButton("Pause");
    QPushButton *slowButton = new QPushButton("0.5x");
    QPushButton *normalButton = new QPushButton("1.0x");
    QPushButton *fastButton = new QPushButton("2.0x");

    QSlider *speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setRange(5, 200);
    speedSlider->setValue(100);

    connect(pauseButton, &QPushButton::clicked, [=]() { setSimulationSpeed(0); });
    connect(slowButton, &QPushButton::clicked, [=]() { setSimulationSpeed(0.5); });
    connect(normalButton, &QPushButton::clicked, [=]() { setSimulationSpeed(1.0); });
    connect(fastButton, &QPushButton::clicked, [=]() { setSimulationSpeed(2.0); });
    connect(speedSlider, &QSlider::valueChanged, this, [=](int value) { setSimulationSpeed(value / 100.0); });

    QHBoxLayout *controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(pauseButton);
    controlsLayout->addWidget(slowButton);
    controlsLayout->addWidget(normalButton);
    controlsLayout->addWidget(fastButton);
    controlsLayout->addWidget(speedSlider);

    // Add the controls layout as a separate widget at the bottom of the main layout
    QWidget *controlsWidget = new QWidget;
    controlsWidget->setLayout(controlsLayout);

    if (centralWidget() && centralWidget()->layout()) {
        QVBoxLayout *mainLayout = static_cast<QVBoxLayout*>(centralWidget()->layout());
        mainLayout->addWidget(controlsWidget);  // Add controls widget below the map
    }
}

void MainWindow::setSimulationSpeed(double speedFactor) {
    simManager->setSpeedFactor(speedFactor);
}

void MainWindow::updateMap() {
    emit simManager->vehiclesUpdated();
}
