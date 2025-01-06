#include "mainwindow.h"
#include "osmimporter.h"
#include <QApplication>
#include <QEventLoop>
#include <QTimer>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Graph graph;
    OSMImporter importer(graph);

    // Bounding box centrée sur Mulhouse
    double minLat = 47.74;
    double minLon = 7.32;
    double maxLat = 47.76;
    double maxLon = 7.34;
    QString bbox = QString("%1,%2,%3,%4").arg(minLat).arg(minLon).arg(maxLat).arg(maxLon);

    // Centre de la bounding box et niveau de zoom
    double centerLat = (minLat + maxLat) / 2;
    double centerLon = (minLon + maxLon) / 2;
    int defaultZoomLevel = 14;

    // Crée une event loop pour attendre que l'import se termine
    QEventLoop loop;
    bool importSuccessful = false;

    // Importe les données centrées sur la bounding box
    importer.importData(bbox);

    // Permet de savoir quand l'import des données OSM est terminé à l'aide du signal finished
    QObject::connect(&importer, &OSMImporter::finished, [&loop, &importSuccessful]() {
        importSuccessful = true;
        loop.quit();
    });

    QTimer::singleShot(30000, &loop, &QEventLoop::quit); // Si l'import prend plus de 30 secondes on quitte l'event loop
    loop.exec(); // Pause l'execution jusqu'à ce que l'event loop soit quittée

    if (!importSuccessful || graph.nodes.isEmpty() || graph.getEdges().isEmpty()) {
        qWarning() << "Erreur lors de l'import des données d'OSM";
        return -1;
    }

    // Lance l'application
    MainWindow wind(&graph, centerLat, centerLon, defaultZoomLevel);
    wind.show();
    return app.exec();
}
