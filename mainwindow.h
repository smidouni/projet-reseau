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
#include <QSpinBox>
#include "simulationmanager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(Graph *graph,
                        double centerLat,
                        double centerLon,
                        int zoomLevel,
                        QWidget *parent = nullptr);

public slots:
    void updateMap();                                 // Mettre à jour la carte
    void setSimulationSpeed(double speedFactor);      // Régler la vitesse de la simulation


private:
    QQuickWidget *mapView;                            // Vue de la carte (QML)
    SimulationManager *simManager;                    // Gestionnaire de simulation
    double centerLat;                                 // Latitude du centre de la carte
    double centerLon;                                 // Longitude du centre de la carte
    int zoomLevel;                                    // Niveau de zoom de la carte

    QGraphicsScene *scene;                            // Scène graphique pour les véhicules
    QTextEdit *logArea;                               // Zone de logs pour afficher les messages
    QLineEdit *messageInput;                          // Champ de saisie pour envoyer des messages
    QComboBox *vehicleSelector;                       // Menu déroulant pour sélectionner les véhicules
    QPushButton *sendButton;        // Bouton pour envoyer un message

    void setupUI();                                   // Configurer l'interface utilisateur
    void setupMap();                                  // Configurer la carte
    void setupControls();                             // Configurer les contrôles (boutons, sliders)
    void setupScene();                                // Configurer la scène graphique

    QSpinBox *vehicleCountSpinBox;  // Champ pour choisir le nombre de véhicules
    QPushButton *resetButton;        // Bouton pour relancer la simulation

    bool isPaused;                   // Indicates if the simulation is paused
    double currentSpeed;             // Current simulation speed factor

    void resetSimulation();        // Méthode pour relancer la simulation

};

#endif // MAINWINDOW_H
