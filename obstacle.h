#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <QObject>
#include <QGeoCoordinate>
#include <QDebug>

/**
 * @brief The Obstacle class
 * Represents a static obstacle on the map that can block edges or be discovered by cars.
 */
class Obstacle : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double lat READ lat NOTIFY positionChanged)
    Q_PROPERTY(double lon READ lon NOTIFY positionChanged)
    Q_PROPERTY(bool isFlashing READ isFlashing NOTIFY flashingChanged)

public:
    explicit Obstacle(int id, double lat, double lon, QObject *parent = nullptr);

    double lat() const { return coordinate.latitude(); }
    double lon() const { return coordinate.longitude(); }
    bool isFlashing() const { return flashing; }

    int getId() const { return id; }

    /**
     * @brief flash
     * Make the obstacle flash green for a short time (UI effect).
     */
    void flash();

signals:
    void positionChanged();
    void flashingChanged();

private:
    int id;
    QGeoCoordinate coordinate;
    bool flashing = false;
};

#endif // OBSTACLE_H
