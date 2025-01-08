#include "osmimporter.h"
#include <QXmlStreamReader>
#include <QDebug>

OSMImporter::OSMImporter(Graph &graph, QObject *parent)
    : QObject(parent), graph(graph)
{
}

void OSMImporter::importData(const QString &bbox)
{
    // Overpass API query to retrieve only "highway" tagged ways within the bounding box
    QString query = QString(
                        "[out:xml];"
                        "("
                        "   way[\"highway\"](%1);"
                        "   >;"
                        ");"
                        "out body;"
                        ).arg(bbox);

    QByteArray queryData = query.toUtf8();
    QUrl url("http://overpass-api.de/api/interpreter");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");

    // Connect the network reply to the handler
    connect(&networkManager, &QNetworkAccessManager::finished,
            this, &OSMImporter::handleNetworkReply);

    networkManager.post(request, queryData);
}

void OSMImporter::handleNetworkReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Erreur réseau:" << reply->errorString();
        reply->deleteLater();
        emit finished();
        return;
    }

    QByteArray data = reply->readAll();
    QXmlStreamReader xml(data);
    parseXml(xml);

    if (xml.hasError()) {
        qWarning() << "Erreur XML durant l'import des données OSM:"
                   << xml.errorString();
    } else {
        qDebug() << "Succès de l'import avec"
                 << graph.nodes.size() << "nodes et"
                 << graph.getEdges().size() << "edges.";
    }

    reply->deleteLater();
    emit finished();
}

void OSMImporter::parseXml(QXmlStreamReader &xml)
{
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

                while (!(xml.tokenType() == QXmlStreamReader::EndElement
                         && xml.name() == "way"))
                {
                    if (xml.tokenType() == QXmlStreamReader::StartElement
                        && xml.name() == "nd")
                    {
                        qint64 ref = xml.attributes().value("ref").toLongLong();
                        if (parsedNodes.contains(ref)) {
                            wayNodes.append(parsedNodes[ref]);
                        }
                    }
                    xml.readNext();
                }

                // Add bidirectional edges between successive node pairs
                for (int i = 0; i < wayNodes.size() - 1; ++i) {
                    Node *start = wayNodes[i];
                    Node *end   = wayNodes[i + 1];

                    // **Skip adding edge if start and end nodes are the same**
                    if (start->id == end->id) {
                        qDebug() << "Skipping duplicate edge between node" << start->id;
                        continue;
                    }

                    double length = start->coordinate.distanceTo(end->coordinate);
                    graph.addEdge(start->id, end->id, length);
                }
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "Erreur pendant le parsing du XML:"
                   << xml.errorString();
    }
}
