#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QQmlContext>
#include <QUrl>
#include <QPushButton>
#include <QSlider>
#include <QRandomGenerator>
#include <QLabel>
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

    commManager = new CommunicationManager(vehicleList, this);

    if (!commManager) {
        qWarning() << "commManager n'a pas été initialisé correctement.";
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

    // Connexion du bouton Pause
    connect(pauseButton, &QPushButton::clicked, [=]() {
        isPaused = !isPaused; // Inverse l'état de pause

        if (isPaused) {
            // Enregistrer la vitesse actuelle et mettre en pause
            currentSpeed = speedSlider->value() / 100.0;
            setSimulationSpeed(0); // Mettre la simulation en pause
            pauseButton->setText("Play");
        } else {
            // Reprendre avec la vitesse sauvegardée
            setSimulationSpeed(currentSpeed);
            pauseButton->setText("Pause");
        }
    });

    // Connexions des boutons de vitesse
    connect(slowButton, &QPushButton::clicked, [=]() {
        currentSpeed = 0.5;
        if (!isPaused) {
            setSimulationSpeed(currentSpeed);
        }
        speedSlider->setValue(static_cast<int>(currentSpeed * 100));
        speedLabel->setText("50%");
    });

    connect(mediumButton, &QPushButton::clicked, [=]() {
        currentSpeed = 1.5;
        if (!isPaused) {
            setSimulationSpeed(currentSpeed);
        }
        speedSlider->setValue(static_cast<int>(currentSpeed * 100));
        speedLabel->setText("150%");
    });

    connect(fastButton, &QPushButton::clicked, [=]() {
        currentSpeed = 2.0;
        if (!isPaused) {
            setSimulationSpeed(currentSpeed);
        }
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

void MainWindow::updateMap() {
    emit simManager->vehiclesUpdated();
}

void MainWindow::handleMessage(Vehicle *from, Vehicle *to, const QString &message) {
    if (!from || !to) {
        qWarning() << "Message reçu avec un pointeur nul.";
        return;
    }

    logArea->append(QString("Vehicle %1 → Vehicle %2: %3")
                        .arg(from->getId())
                        .arg(to->getId())
                        .arg(message));

    QLineF line(QPointF(from->lon(), from->lat()), QPointF(to->lon(), to->lat()));
    QGraphicsLineItem *lineItem = scene->addLine(line, QPen(Qt::red, 2));
    messageLines.append(lineItem);
}

void MainWindow::sendMessage() {
    QString message = messageInput->text().trimmed();
    if (message.isEmpty()) {
        logArea->append("Erreur : Aucun message saisi !");
        return;
    }

    int index = vehicleSelector->currentIndex();
    if (index < 0) {
        logArea->append("Erreur : Aucun véhicule sélectionné !");
        return;
    }

    Vehicle *sender = qobject_cast<Vehicle*>(vehicleSelector->currentData().value<QObject*>());
    if (!sender) {
        logArea->append("Erreur : Émetteur invalide !");
        return;
    }

    logArea->append(QString("Envoi : Vehicle %1 → Tous : %2")
                        .arg(sender->getId())
                        .arg(message));
    commManager->sendMessage(sender, message);
}

void MainWindow::clearMessageLines() {
    for (QGraphicsLineItem *line : messageLines) {
        scene->removeItem(line);
        delete line;
    }
    messageLines.clear();
}

