#ifndef COMMUNICATIONLINKSMODEL_H
#define COMMUNICATIONLINKSMODEL_H

#include <QAbstractListModel>
#include <QGeoCoordinate>

struct CommunicationLink {
    QGeoCoordinate start;
    QGeoCoordinate end;
};

class CommunicationLinksModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum CommunicationLinkRoles {
        StartLatRole = Qt::UserRole + 1,
        StartLonRole,
        EndLatRole,
        EndLonRole
    };

    explicit CommunicationLinksModel(QObject *parent = nullptr);

    // Overrides
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Role names
    QHash<int, QByteArray> roleNames() const override;

    // Update the model
    void setCommunicationLinks(const QList<CommunicationLink> &links);

private:
    QList<CommunicationLink> m_links;
};

#endif // COMMUNICATIONLINKSMODEL_H
