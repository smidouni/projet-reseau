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
#include <QLabel>
#include "simulationmanager.h"
#include "communicationmanager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(Graph *graph, double centerLat, double centerLon, int zoomLevel, QWidget *parent = nullptr);

public slots:
    void updateMap();                                 // Mettre à jour la carte
    void setSimulationSpeed(double speedFactor);      // Régler la vitesse de la simulation
    void handleMessage(Vehicle *from, Vehicle *to, const QString &message); // Gérer les messages entre véhicules
    void sendMessage();                               // Envoyer un message
    void clearMessageLines();                         // Effacer les lignes de communication sur la carte

private:
    QQuickWidget *mapView;                            // Vue de la carte (QML)
    SimulationManager *simManager;                    // Gestionnaire de simulation
    CommunicationManager *commManager;                // Gestionnaire de communication
    double centerLat;                                 // Latitude du centre de la carte
    double centerLon;                                 // Longitude du centre de la carte
    int zoomLevel;                                    // Niveau de zoom de la carte

    QGraphicsScene *scene;                            // Scène graphique pour les véhicules
    QList<QGraphicsLineItem*> messageLines;           // Lignes pour représenter les communications
    QTextEdit *logArea;                               // Zone de logs pour afficher les messages
    QLineEdit *messageInput;                          // Champ de saisie pour envoyer des messages
    QComboBox *vehicleSelector;                       // Menu déroulant pour sélectionner les véhicules
    QPushButton *sendButton;                          // Bouton pour envoyer un message

    void setupUI();                                   // Configurer l'interface utilisateur
    void setupMap();                                  // Configurer la carte
    void setupControls();                             // Configurer les contrôles (boutons, sliders)
    void setupScene();                                // Configurer la scène graphique
};

#endif // MAINWINDOW_H
