#include "communicationlinksmodel.h"

CommunicationLinksModel::CommunicationLinksModel(QObject *parent)
    : QAbstractListModel(parent) {}

int CommunicationLinksModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return m_links.size();
}

QVariant CommunicationLinksModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_links.size())
        return QVariant();

    const CommunicationLink &link = m_links.at(index.row());

    switch (role) {
    case StartLatRole:
        return link.start.latitude();
    case StartLonRole:
        return link.start.longitude();
    case EndLatRole:
        return link.end.latitude();
    case EndLonRole:
        return link.end.longitude();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> CommunicationLinksModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[StartLatRole] = "startLat";
    roles[StartLonRole] = "startLon";
    roles[EndLatRole] = "endLat";
    roles[EndLonRole] = "endLon";
    return roles;
}

void CommunicationLinksModel::setCommunicationLinks(const QList<CommunicationLink> &links) {
    beginResetModel();
    m_links = links;
    endResetModel();
}
