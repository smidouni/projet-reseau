#include "mainwindow.h"
#include "osmimporter.h"
#include <QApplication>
#include <QEventLoop>
#include <QTimer>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Graph graph;

    // Create an OSMImporter instance and import OSM data using a smaller bounding box for central Mulhouse.
    OSMImporter importer(graph);

    // Bounding box for a central area in Mulhouse
    double minLat = 47.74;
    double minLon = 7.32;
    double maxLat = 47.76;
    double maxLon = 7.34;
    QString bbox = QString("%1,%2,%3,%4").arg(minLat).arg(minLon).arg(maxLat).arg(maxLon);

    // Calculate the center and approximate zoom level
    double centerLat = (minLat + maxLat) / 2;
    double centerLon = (minLon + maxLon) / 2;
    int defaultZoomLevel = 14;  // Adjust as necessary to fit the entire area

    // Create an event loop to wait until the import completes
    QEventLoop loop;
    bool importSuccessful = false;

    importer.importData(bbox);

    // Connect to the finished signal to handle completion of data import
    QObject::connect(&importer, &OSMImporter::finished, [&loop, &importSuccessful]() {
        importSuccessful = true;
        loop.quit();
    });

    QTimer::singleShot(30000, &loop, &QEventLoop::quit); // 30-second timeout
    loop.exec();

    if (!importSuccessful || graph.nodes.isEmpty() || graph.getEdges().isEmpty()) {
        qWarning() << "Graph import failed or resulted in an empty graph. Exiting.";
        return -1;
    }

    MainWindow w(&graph, centerLat, centerLon, defaultZoomLevel); // Pass the center and zoom level
    w.show();

    return a.exec();
}
