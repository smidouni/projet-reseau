#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QQmlContext>
#include <QUrl>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QRandomGenerator>
#include <qgraphicsitem.h>
#include "communicationmanager.h"


MainWindow::MainWindow(Graph *graph, double centerLat, double centerLon, int zoomLevel, QWidget *parent)
    : QMainWindow(parent), simManager(new SimulationManager(*graph)), centerLat(centerLat), centerLon(centerLon), zoomLevel(zoomLevel) {
    // Mise en place de l'UI
    setupUI();

    // Mise en place de la carte
    setupMap();

    // Ajoute des boutons à l'UI et de leurs events
    setupControls();

    // On genere 40 vehicules
    for (int i = 0; i < 200; ++i) {
        qint64 startNodeId = graph->nodes.keys().at(QRandomGenerator::global()->bounded(graph->nodes.size()));
        simManager->addVehicle(i, startNodeId);
    }

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

    // updateMap est lancée quand simManager emit "updated"
    connect(simManager, &SimulationManager::updated, this, &MainWindow::updateMap);
    connect(commManager, &CommunicationManager::messageSent, this, &MainWindow::handleMessage);
    //connect(sendButton, &QPushButton::clicked, this, &MainWindow::sendMessage);

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

    // Ajoute les boutons et slider en bas

    QWidget *controlsWidget = new QWidget;
    controlsWidget->setLayout(controlsLayout);

    if (centralWidget() && centralWidget()->layout()) {
        QVBoxLayout *mainLayout = static_cast<QVBoxLayout*>(centralWidget()->layout());
        mainLayout->addWidget(controlsWidget);
    }

    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Entrez le message à envoyer");

    // Menu déroulant pour choisir le véhicule émetteur

    QList<Vehicle*> vehicleList;
    for (QObject *obj : simManager->getVehicles()) {
        Vehicle *vehicle = qobject_cast<Vehicle*>(obj);
        if (vehicle) {
            vehicleList.append(vehicle);
        }
    }

    vehicleSelector = new QComboBox(this);
    for (Vehicle *vehicle : vehicleList) {
        vehicleSelector->addItem(QString("Vehicle %1").arg(vehicle->getId()), QVariant::fromValue(vehicle));
    }

    // Bouton pour envoyer le message
    sendButton = new QPushButton("Envoyer le message", this);
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::sendMessage);

    // Ajout des widgets à un layout vertical
    QVBoxLayout *controlLayout = new QVBoxLayout;
    controlLayout->addWidget(new QLabel("Message :"));
    controlLayout->addWidget(messageInput);
    controlLayout->addWidget(new QLabel("Émetteur :"));
    controlLayout->addWidget(vehicleSelector);
    controlLayout->addWidget(sendButton);

    // Ajout du layout dans la fenêtre principale
    QWidget *controlWidget = new QWidget(this);
    controlWidget->setLayout(controlLayout);

    // Ajout des contrôles au bas de la fenêtre
    QVBoxLayout *mainLayout = static_cast<QVBoxLayout*>(centralWidget()->layout());
    mainLayout->addWidget(controlWidget);
}

void MainWindow::setSimulationSpeed(double speedFactor) {
    simManager->setSpeedFactor(speedFactor);
}

void MainWindow::updateMap() {
    emit simManager->vehiclesUpdated();
}

void MainWindow::setupScene() {
    for (QObject *obj : simManager->getVehicles()) {
        Vehicle *vehicle = qobject_cast<Vehicle*>(obj); // Conversion explicite
        if (!vehicle) continue; // Vérifiez que le cast est valide
        // Couleur aléatoire pour le cercle de portée
        int red = QRandomGenerator::global()->bounded(256);
        int green = QRandomGenerator::global()->bounded(256);
        int blue = QRandomGenerator::global()->bounded(256);
        QColor randomColor(red, green, blue, 50);

        // Cercle de communication
        QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(
            vehicle->lat() - vehicle->getCommunicationRange(),
            vehicle->lon() - vehicle->getCommunicationRange(),
            vehicle->getCommunicationRange() * 2,
            vehicle->getCommunicationRange() * 2
            );
        circle->setBrush(QBrush(randomColor));
        scene->addItem(circle);

        // Rectangle représentant le véhicule
        QGraphicsRectItem *rect = new QGraphicsRectItem(vehicle->lat() - 5, vehicle->lon() - 5, 10, 10);
        rect->setBrush(Qt::blue);
        scene->addItem(rect);

        // Texte pour afficher l'identifiant du véhicule
        QGraphicsTextItem *text = new QGraphicsTextItem(QString::number(vehicle->getId()));
        text->setPos(vehicle->lat() + 10, vehicle->lon() - 10);
        scene->addItem(text);
    }
}

void MainWindow::handleMessage(Vehicle *from, Vehicle *to, const QString &message) {
    if (!from || !to) {
        qWarning() << "Message reçu avec un pointeur nul.";
        return;
    }

    logArea->append(QString("Vehicle %1 → Vehicle %2: %3").arg(from->getId()).arg(to->getId()).arg(message));

    QLineF line(QPointF(from->lon(), from->lat()), QPointF(to->lon(), to->lat()));
    QGraphicsLineItem *lineItem = scene->addLine(line, QPen(Qt::red, 2));
    messageLines.append(lineItem);
}


void MainWindow::sendMessage() {
    // Récupérer le message saisi
    QString message = messageInput->text().trimmed();
    if (message.isEmpty()) {
        logArea->append("Erreur : Aucun message saisi !");
        return;
    }

    // Récupérer le véhicule sélectionné
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

    // Envoyer le message via CommunicationManager
    logArea->append(QString("Envoi : Vehicle %1 → Tous : %2").arg(sender->getId()).arg(message));
    commManager->sendMessage(sender, message);
}


void MainWindow::clearMessageLines() {
    for (QGraphicsLineItem *line : messageLines) {
        scene->removeItem(line);
        delete line;
    }
    messageLines.clear();
}

