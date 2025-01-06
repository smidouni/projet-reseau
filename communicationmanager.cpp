#include "communicationmanager.h"
#include <QtMath>

CommunicationManager::CommunicationManager(QList<Vehicle*> vehicles, QObject *parent)
    : QObject(parent), vehicles(vehicles) {}

void CommunicationManager::sendMessage(Vehicle *sender, const QString &message) {
    qDebug() << "Message envoyé par Vehicle" << sender->getId() << ":" << message;

    QSet<Vehicle*> visited;               // Ensemble pour éviter les boucles
    QList<Vehicle*> queue;                // File d'attente pour la propagation

    queue.append(sender);                 // Ajouter le véhicule émetteur à la file
    visited.insert(sender);               // Marquer comme visité

    while (!queue.isEmpty()) {
        Vehicle *current = queue.takeFirst(); // Récupérer le premier élément de la file

        // Parcourir les autres véhicules
        for (Vehicle *vehicle : vehicles) {
            if (vehicle == current || visited.contains(vehicle)) continue;

            // Calculer la distance
            double distance = qSqrt(qPow(current->lat() - vehicle->lat(), 2) +
                                    qPow(current->lon() - vehicle->lon(), 2));

            if (distance <= current->getCommunicationRange()) {
                // Envoi du message
                emit messageSent(current, vehicle, message);
                vehicle->receiveMessage(message);

                // Ajouter le véhicule à la file pour continuer la propagation
                queue.append(vehicle);
                visited.insert(vehicle);
            }
        }
    }
}
