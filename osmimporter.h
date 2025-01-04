#ifndef OSMIMPORTER_H
#define OSMIMPORTER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include "graph.h"

class OSMImporter : public QObject {
    Q_OBJECT

public:
    explicit OSMImporter(Graph &graph, QObject *parent = nullptr);
    void importData(const QString &bbox);

signals:
    void finished();  // Signal qui indique que l'import est terminé

private slots:
    void handleNetworkReply(QNetworkReply *reply);

private:
    Graph &graph;
    QNetworkAccessManager networkManager;
    void parseXml(QXmlStreamReader &xml);
};

#endif // OSMIMPORTER_H
