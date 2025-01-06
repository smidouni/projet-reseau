#include "mainwindow.h"
#include "osmimporter.h"
#include <QApplication>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Graph fullGraph;
    OSMImporter importer(fullGraph);

    // Example bounding box:
    double minLat = 47.74;
    double minLon = 7.32;
    double maxLat = 47.76;
    double maxLon = 7.34;
    QString bbox = QString("%1,%2,%3,%4").arg(minLat).arg(minLon).arg(maxLat).arg(maxLon);

    double centerLat = (minLat + maxLat) / 2.0;
    double centerLon = (minLon + maxLon) / 2.0;
    int defaultZoomLevel = 14;

    QEventLoop loop;
    bool importSuccessful = false;

    importer.importData(bbox);

    QObject::connect(&importer, &OSMImporter::finished, [&loop, &importSuccessful]() {
        importSuccessful = true;
        loop.quit();
    });

    QTimer::singleShot(30000, &loop, &QEventLoop::quit);
    loop.exec();

    if (!importSuccessful || fullGraph.nodes.isEmpty() || fullGraph.getEdges().isEmpty()) {
        qWarning() << "Erreur lors de l'import des données OSM";
        return -1;
    }

    // Build simplified graph if you want to speed up pathfinding
    Graph simplifiedGraph = fullGraph.createSimplifiedGraph();
    qDebug() << "Simplified graph: " << simplifiedGraph.nodes.size()
             << "nodes," << simplifiedGraph.getEdges().size() << "edges.";

    MainWindow w(&simplifiedGraph, centerLat, centerLon, defaultZoomLevel);
    w.show();

    return app.exec();
}
