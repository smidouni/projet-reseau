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
#include <qgraphicsitem.h>
#include "communicationmanager.h"

// Variables globales pour la vitesse et l'état de pause
double currentSpeed = 1.0; // Vitesse par défaut : 1.0x
bool isPaused = false;     // Indique si la simulation est en pause

MainWindow::MainWindow(Graph *graph, double centerLat, double centerLon, int zoomLevel, QWidget *parent)
    : QMainWindow(parent), simManager(new SimulationManager(*graph)), centerLat(centerLat), centerLon(centerLon), zoomLevel(zoomLevel) {
    // Mise en place de l'UI
    setupUI();

    // Mise en place de la carte
    setupMap();

    // Ajoute des boutons à l'UI et leurs événements
    setupControls();

    // Génère 3 véhicules au départ
    for (int i = 0; i < 3; ++i) {
        qint64 startNodeId = graph->nodes.keys().at(QRandomGenerator::global()->bounded(graph->nodes.size()));
        simManager->addVehicle(i, startNodeId);
    }

    // Vérifie l'initialisation du CommunicationManager
    QList<Vehicle*> vehicleList;
    for (QObject *obj : simManager->getVehicles()) {
        Vehicle *vehicle = qobject_cast<Vehicle*>(obj);
        if (vehicle) {
            vehicleList.append(vehicle);
        }
    }

}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Initialisation de la carte
    mapView = new QQuickWidget;
    mapView->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mapView->setMinimumSize(800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(mapView);
    centralWidget->setLayout(mainLayout);
}

void MainWindow::setupMap() {
    QQmlContext *context = mapView->rootContext();
    context->setContextProperty("simManager", simManager);
    context->setContextProperty("initialCenterLat", centerLat);
    context->setContextProperty("initialCenterLon", centerLon);
    context->setContextProperty("initialZoomLevel", zoomLevel);

    mapView->setSource(QUrl(QStringLiteral("qrc:/maCarte/MapView.qml")));
}

void MainWindow::setupControls() {
    // Bouton Pause
    QPushButton *pauseButton = new QPushButton("Pause");

    // Boutons de vitesse
    QPushButton *slowButton = new QPushButton("0.5x");
    QPushButton *mediumButton = new QPushButton("1.5x");
    QPushButton *fastButton = new QPushButton("2.0x");

    // Slider pour ajuster la vitesse
    QSlider *speedSlider = new QSlider(Qt::Horizontal);
    QLabel *speedLabel = new QLabel("100%", this);

    speedSlider->setRange(5, 200); // Plage de 5% à 200%
    speedSlider->setValue(100);    // Valeur initiale : 100%

    // SpinBox pour choisir le nombre de véhicules
    QLabel *vehicleCountLabel = new QLabel("Nombre de voitures :", this);
    vehicleCountSpinBox = new QSpinBox(this);
    vehicleCountSpinBox->setRange(1, 100); // Plage entre 1 et 100 véhicules
    vehicleCountSpinBox->setValue(3);      // Valeur par défaut

    // Bouton pour relancer la simulation
    resetButton = new QPushButton("Relancer la simulation", this);

    // Connexion pour le bouton Reset
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetSimulation);

    // Connexion du bouton Pause
    connect(pauseButton, &QPushButton::clicked, [=]() {
        isPaused = !isPaused; // Inverse l'état de pause

        if (isPaused) {
            currentSpeed = speedSlider->value() / 100.0;
            setSimulationSpeed(0); // Mettre la simulation en pause
            pauseButton->setText("Play");
        } else {
            setSimulationSpeed(currentSpeed);
            pauseButton->setText("Pause");
        }
    });

    // Connexions des boutons de vitesse
    connect(slowButton, &QPushButton::clicked, [=]() {
        currentSpeed = 0.5;
        if (!isPaused) setSimulationSpeed(currentSpeed);
        speedSlider->setValue(static_cast<int>(currentSpeed * 100));
        speedLabel->setText("50%");
    });

    connect(mediumButton, &QPushButton::clicked, [=]() {
        currentSpeed = 1.5;
        if (!isPaused) setSimulationSpeed(currentSpeed);
        speedSlider->setValue(static_cast<int>(currentSpeed * 100));
        speedLabel->setText("150%");
    });

    connect(fastButton, &QPushButton::clicked, [=]() {
        currentSpeed = 2.0;
        if (!isPaused) setSimulationSpeed(currentSpeed);
        speedSlider->setValue(static_cast<int>(currentSpeed * 100));
        speedLabel->setText("200%");
    });

    // Connexion du slider
    connect(speedSlider, &QSlider::valueChanged, this, [=](int value) {
        if (!isPaused) {
            double speed = value / 100.0;
            setSimulationSpeed(speed);
            currentSpeed = speed; // Sauvegarder la vitesse actuelle
        } else {
            currentSpeed = value / 100.0;
        }
        speedLabel->setText(QString::number(value) + "%");
    });

    // Disposition des contrôles
    QHBoxLayout *controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(pauseButton);
    controlsLayout->addWidget(slowButton);
    controlsLayout->addWidget(mediumButton);
    controlsLayout->addWidget(fastButton);
    controlsLayout->addWidget(speedSlider);
    controlsLayout->addWidget(speedLabel);
    controlsLayout->addWidget(vehicleCountLabel);
    controlsLayout->addWidget(vehicleCountSpinBox);
    controlsLayout->addWidget(resetButton);

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

        // Vérification du graphe
        if (simManager->getGraph().nodes.isEmpty()) {
            qCritical() << "Le graphe est vide. Impossible de relancer la simulation.";
            return;
        }

        // Suppression des véhicules existants
        simManager->clearVehicles();

        // Ajout de nouveaux véhicules
        int numVehicles = vehicleCountSpinBox->value();
        for (int i = 0; i < numVehicles; ++i) {
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
    emit simManager->vehiclesUpdated(); // Assurez-vous que cette méthode est valide
}
