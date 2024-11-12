#include "osmimporter.h"
#include <QXmlStreamReader>
#include <QDebug>

OSMImporter::OSMImporter(Graph &graph, QObject *parent)
    : QObject(parent), graph(graph) { }

void OSMImporter::importData(const QString &bbox) {
    QString query = QString("[out:xml];(node(%1);way(%1);relation(%1););out body;").arg(bbox);
    QByteArray queryData = query.toUtf8();
    QUrl url("http://overpass-api.de/api/interpreter");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    // Lance handleNetworkReply quand les données sont importées
    connect(&networkManager, &QNetworkAccessManager::finished, this, &OSMImporter::handleNetworkReply);

    networkManager.post(request, queryData);
}

void OSMImporter::handleNetworkReply(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Erreur reseau:" << reply->errorString();
        reply->deleteLater();
        emit finished();  // Emit finished signal even on error
        return;
    }

    QByteArray data = reply->readAll();
    QXmlStreamReader xml(data);
    parseXml(xml);

    if (xml.hasError()) {
        qWarning() << "Erreur XML durant l'import des données OSM:" << xml.errorString();
    } else {
        qDebug() << "Succès de l'import avec" << graph.nodes.size() << "nodes et" << graph.getEdges().size() << "edges.";
    }

    reply->deleteLater();
    emit finished();
}

void OSMImporter::parseXml(QXmlStreamReader &xml) {
    QMap<qint64, Node*> parsedNodes;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "node") {
                qint64 id = xml.attributes().value("id").toLongLong();
                double lat = xml.attributes().value("lat").toDouble();
                double lon = xml.attributes().value("lon").toDouble();
                QGeoCoordinate coord(lat, lon);
                Node *node = new Node(id, coord);
                parsedNodes.insert(id, node);
                graph.addNode(id, coord);
            } else if (xml.name() == "way") {
                QList<Node*> wayNodes;
                while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "way")) {
                    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "nd") {
                        qint64 ref = xml.attributes().value("ref").toLongLong();
                        if (parsedNodes.contains(ref)) {
                            wayNodes.append(parsedNodes[ref]);
                        }
                    }
                    xml.readNext();
                }

                for (int i = 0; i < wayNodes.size() - 1; ++i) {
                    Node *start = wayNodes[i];
                    Node *end = wayNodes[i + 1];
                    double length = start->coordinate.distanceTo(end->coordinate);
                    graph.addEdge(start->id, end->id, length);
                }
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "Erreur pendant le parsing du XML:" << xml.errorString();
    }
}
