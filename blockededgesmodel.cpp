#include "blockededgesmodel.h"

BlockedEdgesModel::BlockedEdgesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int BlockedEdgesModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_blockedEdges.size();
}

QVariant BlockedEdgesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_blockedEdges.size())
        return QVariant();

    const BlockedEdge &edge = m_blockedEdges.at(index.row());

    switch (role) {
    case StartLatRole:
        return edge.startLat;
    case StartLonRole:
        return edge.startLon;
    case EndLatRole:
        return edge.endLat;
    case EndLonRole:
        return edge.endLon;
    case StartIdRole:
        return edge.startId;
    case EndIdRole:
        return edge.endId;
    case BlockedAtRole:
        return edge.blockedAt;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> BlockedEdgesModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[StartLatRole] = "startLat";
    roles[StartLonRole] = "startLon";
    roles[EndLatRole] = "endLat";
    roles[EndLonRole] = "endLon";
    roles[StartIdRole] = "startId";
    roles[EndIdRole] = "endId";
    roles[BlockedAtRole] = "blockedAt";
    return roles;
}

void BlockedEdgesModel::updateBlockedEdges(const QList<QPair<qint64, qint64>> &blockedEdges, const QMap<QPair<qint64, qint64>, Edge*> &edges) {
    beginResetModel();
    m_blockedEdges.clear();

    for (const auto &pair : blockedEdges) {
        Edge *edge = edges.value(pair, nullptr);
        if (edge) {
            BlockedEdge be;
            be.startLat = edge->start->coordinate.latitude();
            be.startLon = edge->start->coordinate.longitude();
            be.endLat = edge->end->coordinate.latitude();
            be.endLon = edge->end->coordinate.longitude();
            be.blockedAt = QDateTime::currentDateTime(); // Set current time
            be.startId = edge->start->id;
            be.endId = edge->end->id;
            m_blockedEdges.append(be);
        }
    }

    endResetModel();
}

void BlockedEdgesModel::addBlockedEdgeWithTimestamp(qint64 startId, qint64 endId, double startLat, double startLon, double endLat, double endLon, const QDateTime &timestamp)
{
    BlockedEdge be;
    be.startId = startId;
    be.endId = endId;
    be.startLat = startLat;
    be.startLon = startLon;
    be.endLat = endLat;
    be.endLon = endLon;
    be.blockedAt = timestamp;

    beginInsertRows(QModelIndex(), m_blockedEdges.size(), m_blockedEdges.size());
    m_blockedEdges.append(be);
    endInsertRows();
}

void BlockedEdgesModel::removeBlockedEdge(qint64 startId, qint64 endId)
{
    for (int i = 0; i < m_blockedEdges.size(); ++i) {
        const BlockedEdge &edge = m_blockedEdges.at(i);
        if ((edge.startId == startId && edge.endId == endId) ||
            (edge.startId == endId && edge.endId == startId)) {
            beginRemoveRows(QModelIndex(), i, i);
            m_blockedEdges.removeAt(i);
            endRemoveRows();
            break;
        }
    }
}

QVariantList BlockedEdgesModel::getBlockedEdges() const {
    QVariantList list;
    for (const BlockedEdge &edge : m_blockedEdges) {
        QVariantMap map;
        map["startLat"] = edge.startLat;
        map["startLon"] = edge.startLon;
        map["endLat"] = edge.endLat;
        map["endLon"] = edge.endLon;
        map["startId"] = edge.startId;
        map["endId"] = edge.endId;
        map["blockedAt"] = edge.blockedAt.toString(Qt::ISODate);
        list.append(map);
    }
    return list;
}

