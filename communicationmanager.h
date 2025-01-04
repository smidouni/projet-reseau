#ifndef COMMUNICATIONMANAGER_H
#define COMMUNICATIONMANAGER_H

#include <QObject>
#include <QSet>
#include "vehicle.h"

class CommunicationManager : public QObject {
    Q_OBJECT

public:
    explicit CommunicationManager(QList<Vehicle*> vehicles, QObject *parent = nullptr);

    void sendMessage(Vehicle *sender, const QString &message);

signals:
    void messageSent(Vehicle *from, Vehicle *to, const QString &message);

private:
    QList<Vehicle*> vehicles;
};

#endif // COMMUNICATIONMANAGER_H
