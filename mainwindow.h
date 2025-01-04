#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QQuickWidget>
#include <QGraphicsScene>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include "simulationmanager.h"
#include "communicationmanager.h"


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(Graph *graph, double centerLat, double centerLon, int zoomLevel, QWidget *parent = nullptr);

public slots:
    void updateMap();
    void setSimulationSpeed(double speedFactor);

    void handleMessage(Vehicle *from, Vehicle *to, const QString &message); // Gérer les messages
    void sendMessage();                                                     // Envoyer un message
    void clearMessageLines();                                               // Effacer les lignes

private:
    QQuickWidget *mapView;
    SimulationManager *simManager;
    double centerLat;
    double centerLon;
    int zoomLevel;

    QGraphicsScene *scene;               // Scène graphique pour les véhicules
    QList<QGraphicsLineItem*> messageLines; // Lignes pour représenter les messages
    QTextEdit *logArea;                  // Zone de logs pour afficher les messages
    QLineEdit *messageInput;             // Entrée pour le message
    QComboBox *vehicleSelector;          // Menu déroulant pour sélectionner le véhicule
    QPushButton *sendButton;             // Bouton pour envoyer le message
    CommunicationManager *commManager; // Gestionnaire des messages entre véhicules


    void setupUI();
    void setupMap();
    void setupControls();

    void setupVehicles();
    void setupScene();

};

#endif // MAINWINDOW_H
