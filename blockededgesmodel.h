#ifndef BLOCKEDEDGESMODEL_H
#define BLOCKEDEDGESMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include "graph.h"

struct BlockedEdge {
    double startLat;
    double startLon;
    double endLat;
    double endLon;
    QDateTime blockedAt; // Timestamp when the edge was blocked
    qint64 startId;      // Node IDs for unblocking
    qint64 endId;
};

class BlockedEdgesModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum BlockedEdgeRoles {
        StartLatRole = Qt::UserRole + 1,
        StartLonRole,
        EndLatRole,
        EndLonRole,
        StartIdRole,
        EndIdRole,
        BlockedAtRole,
        FlashingRole
    };

    explicit BlockedEdgesModel(QObject *parent = nullptr);

    // Overrides
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Role names
    QHash<int, QByteArray> roleNames() const override;

    // Methods to update the model
    void updateBlockedEdges(const QList<QPair<qint64, qint64>> &blockedEdges, const QMap<QPair<qint64, qint64>, Edge*> &edges);
    void addBlockedEdgeWithTimestamp(qint64 startId, qint64 endId, double startLat, double startLon, double endLat, double endLon, const QDateTime &timestamp);
    void removeBlockedEdge(qint64 startId, qint64 endId);
    QVariantList getBlockedEdges() const;

private:
    QList<BlockedEdge> m_blockedEdges;
};

#endif // BLOCKEDEDGESMODEL_H
