#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QQuickWidget>
#include <QPushButton>
#include <QSlider>
#include "SimulationManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(Graph *graph, double centerLat, double centerLon, int zoomLevel, QWidget *parent = nullptr);

public slots:
    void updateMap();
    void setSimulationSpeed(double speedFactor);

private:
    QQuickWidget *mapView;
    SimulationManager *simManager;
    double centerLat;
    double centerLon;
    int zoomLevel;

    void setupUI();
    void setupMap();
    void setupControls();
};

#endif // MAINWINDOW_H
