// mainwindow.cpp

#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QQmlContext>
#include <QUrl>
#include <QPushButton>
#include <QSlider>
#include <QRandomGenerator>
#include <QLabel>
#include <QSpinBox>
#include <QDebug>

MainWindow::MainWindow(Graph *graph, double centerLat, double centerLon, int zoomLevel, QWidget *parent)
    : QMainWindow(parent),
    simManager(new SimulationManager(*graph, this)),
    centerLat(centerLat),
    centerLon(centerLon),
    zoomLevel(zoomLevel),
    isPaused(false),
    currentSpeed(1.0)
{
    // Setup UI
    setupUI();

    // Setup Map
    setupMap();

    // Setup Controls
    setupControls();

    // Generate initial vehicles
    for (int i = 0; i < 40; ++i) {
        if (graph->nodes.isEmpty()) {
            qWarning() << "Graph is empty; cannot add vehicle" << i;
            continue;
        }
        qint64 startNodeId = graph->nodes.keys().at(QRandomGenerator::global()->bounded(graph->nodes.size()));
        simManager->addVehicle(i, startNodeId);
    }

    // Connect blockedEdgesChanged signal to update the model in QML
    connect(simManager, &SimulationManager::blockedEdgesChanged, this, [this]() {
        // The BlockedEdgesModel is already updated within SimulationManager
        // and QML is bound to it, so no additional action is needed here.
    });
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Initialize UI Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    centralWidget->setLayout(mainLayout);

    // Initialize Map View
    mapView = new QQuickWidget;
    mapView->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mapView->setMinimumSize(800, 600);

    mainLayout->addWidget(mapView);
}

void MainWindow::setupMap() {
    QQmlContext *context = mapView->rootContext();
    context->setContextProperty("simManager", simManager);
    context->setContextProperty("blockedEdgesModel", simManager->blockedEdgesModel());
    context->setContextProperty("initialCenterLat", centerLat);
    context->setContextProperty("initialCenterLon", centerLon);
    context->setContextProperty("initialZoomLevel", zoomLevel);
    context->setContextProperty("communicationLinks", simManager->getCommunicationLinks());

    mapView->setSource(QUrl(QStringLiteral("qrc:/maCarte/MapView.qml")));
}


void MainWindow::setupControls() {
    // Pause Button
    QPushButton *pauseButton = new QPushButton("Pause");

    // Speed Buttons
    QPushButton *slowButton = new QPushButton("0.5x");
    QPushButton *normalButton = new QPushButton("1.0x");
    QPushButton *mediumButton = new QPushButton("1.5x");
    QPushButton *fastButton = new QPushButton("2.0x");

    // Speed Slider
    QSlider *speedSlider = new QSlider(Qt::Horizontal);
    QLabel *speedLabel = new QLabel("100%", this);

    speedSlider->setRange(5, 10000); // Adjusted range for 0.05x to 100.0x
    speedSlider->setValue(100);    // Default 100%

    // Vehicle Count SpinBox
    QLabel *vehicleCountLabel = new QLabel("Nombre de voitures :", this);
    vehicleCountSpinBox = new QSpinBox(this);
    vehicleCountSpinBox->setRange(1, 100); // 1 to 100 vehicles
    vehicleCountSpinBox->setValue(40);      // Default value

    // Reset Button
    resetButton = new QPushButton("Relancer la simulation", this);

    // **Removed Block Edge Button**

    // Connect Reset Button
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetSimulation);

    // Connect Pause Button
    connect(pauseButton, &QPushButton::clicked, this, [this, speedSlider, pauseButton]() {
        isPaused = !isPaused; // Toggle pause state

        if (isPaused) {
            currentSpeed = speedSlider->value(); // Current speed is saved
            setSimulationSpeed(0); // Pause simulation
            pauseButton->setText("Play");
        } else {
            setSimulationSpeed(currentSpeed / 100.0);
            pauseButton->setText("Pause");
        }
    });

    // Connect Speed Buttons
    connect(slowButton, &QPushButton::clicked, this, [this, speedSlider, speedLabel]() {
        currentSpeed = 50; // 50%
        if (!isPaused) setSimulationSpeed(currentSpeed / 100.0);
        speedSlider->setValue(static_cast<int>(currentSpeed));
        speedLabel->setText("50%");
    });

    connect(normalButton, &QPushButton::clicked, this, [this, speedSlider, speedLabel]() { // 1.0x
        currentSpeed = 100; // 100%
        if (!isPaused) setSimulationSpeed(currentSpeed / 100.0);
        speedSlider->setValue(static_cast<int>(currentSpeed));
        speedLabel->setText("100%");
    });

    connect(mediumButton, &QPushButton::clicked, this, [this, speedSlider, speedLabel]() {
        currentSpeed = 150; // 150%
        if (!isPaused) setSimulationSpeed(currentSpeed / 100.0);
        speedSlider->setValue(static_cast<int>(currentSpeed));
        speedLabel->setText("150%");
    });

    connect(fastButton, &QPushButton::clicked, this, [this, speedSlider, speedLabel]() {
        currentSpeed = 200; // 200%
        if (!isPaused) setSimulationSpeed(currentSpeed / 100.0);
        speedSlider->setValue(static_cast<int>(currentSpeed));
        speedLabel->setText("200%");
    });

    // Connect Speed Slider
    connect(speedSlider, &QSlider::valueChanged, this, [this, speedLabel](int value) {
        if (!isPaused) {
            double speedFactor = value / 100.0;
            setSimulationSpeed(speedFactor);
            currentSpeed = value; // Save current speed
        } else {
            currentSpeed = value; // Update speed factor for when unpausing
        }
        speedLabel->setText(QString::number(value) + "%");
    });

    // **Removed Block Edge Button Connection**

    // Arrange Controls Layout
    QHBoxLayout *controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(pauseButton);
    controlsLayout->addWidget(slowButton);
    controlsLayout->addWidget(normalButton);
    controlsLayout->addWidget(mediumButton);
    controlsLayout->addWidget(fastButton);
    controlsLayout->addWidget(speedSlider);
    controlsLayout->addWidget(speedLabel);
    controlsLayout->addWidget(vehicleCountLabel);
    controlsLayout->addWidget(vehicleCountSpinBox);
    controlsLayout->addWidget(resetButton);
    // **Removed controlsLayout->addWidget(blockEdgeButton);**

    QWidget *controlsWidget = new QWidget;
    controlsWidget->setLayout(controlsLayout);

    if (centralWidget() && centralWidget()->layout()) {
        QVBoxLayout *mainLayout = static_cast<QVBoxLayout*>(centralWidget()->layout());
        mainLayout->addWidget(controlsWidget);
    }
}

void MainWindow::setSimulationSpeed(double speedFactor) {
    simManager->setSpeedFactor(speedFactor);
}

void MainWindow::resetSimulation() {
    try {
        qDebug() << "Début de la réinitialisation de la simulation...";

        // Verify graph
        if (simManager->getGraph().nodes.isEmpty()) {
            qCritical() << "Le graphe est vide. Impossible de relancer la simulation.";
            return;
        }

        // Clear existing vehicles
        simManager->clearVehicles();

        // Add new vehicles
        int numVehicles = vehicleCountSpinBox->value();
        for (int i = 0; i < numVehicles; ++i) {
            if (simManager->getGraph().nodes.isEmpty()) {
                qWarning() << "Graph is empty; cannot add vehicle" << i;
                continue;
            }
            qint64 startNodeId = simManager->getGraph().nodes.keys().at(
                QRandomGenerator::global()->bounded(simManager->getGraph().nodes.size()));
            simManager->addVehicle(i, startNodeId);
        }

        emit simManager->vehiclesUpdated();
        qDebug() << "Simulation relancée avec" << numVehicles << "véhicules.";
    } catch (const std::exception &e) {
        qCritical() << "Exception pendant le reset :" << e.what();
    } catch (...) {
        qCritical() << "Erreur inconnue pendant le reset.";
    }
}

void MainWindow::updateMap() {
    qDebug() << "Mise à jour de la carte...";
    emit simManager->vehiclesUpdated();
}
