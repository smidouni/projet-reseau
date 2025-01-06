import QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 5.15
import QtPositioning 5.15
import QtQuick.Controls 2.15

Rectangle {
    width: 800
    height: 600

    // Plugin OpenStreetMap
    Plugin {
        id: mapPlugin
        name: "osm"
    }

    // Carte principale
    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        zoomLevel: 14
        center: QtPositioning.coordinate(initialCenterLat, initialCenterLon)

        MapItemView {
            id: vehicleView
            model: simManager.vehiclesModel

            delegate: MapQuickItem {
                // The lat/lon for this vehicle
                coordinate: QtPositioning.coordinate(modelData.lat, modelData.lon)

                // We'll anchor so (0,0) is at the center of our gradient
                anchorPoint.x: container.width / 2
                anchorPoint.y: container.height / 2

                sourceItem: Item {
                    id: container
                    width: 512
                    height: 512

                    // The actual car icon on top
                    Image {
                        id: carIcon
                        anchors.centerIn: parent
                        source: "qrc:/images/car_icon.svg"
                        width: 32
                        height: 32
                    }
                }
            }
        }

        MapItemView {
            id: obstacleView
            model: simManager.obstaclesModel

            delegate: MapQuickItem {
                coordinate: QtPositioning.coordinate(modelData.lat, modelData.lon)
                anchorPoint.x: obsImg.width / 2
                anchorPoint.y: obsImg.height / 2

                sourceItem: Image {
                    id: obsImg
                    source: "qrc:/images/obstacle_icon.svg"
                    width: 32
                    height: 32
                }
            }
        }

        // Zone pour les interactions (zoom et déplacement)
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            // Déplacement de la carte par clic et glisser
            property var lastMousePosition
            onPressed: lastMousePosition = Qt.point(mouse.x, mouse.y)
            onPositionChanged: {
                if (mouse.buttons & Qt.LeftButton) {
                    var dx = lastMousePosition.x - mouse.x
                    var dy = lastMousePosition.y - mouse.y
                    lastMousePosition = Qt.point(mouse.x, mouse.y)

                    var metersPerPixel = 156543.03392 * Math.cos(map.center.latitude * Math.PI / 180) / Math.pow(2, map.zoomLevel)
                    var newCenter = QtPositioning.coordinate(
                        map.center.latitude + (dy * metersPerPixel / 111320),
                        map.center.longitude - (dx * metersPerPixel / (111320 * Math.cos(map.center.latitude * Math.PI / 180)))
                    )
                    map.center = newCenter
                }
            }

            // Zoom avec la molette
            onWheel: {
                if (wheel.angleDelta.y > 0 && map.zoomLevel < 20) {
                    map.zoomLevel += 0.5
                } else if (wheel.angleDelta.y < 0 && map.zoomLevel > 12) {
                    map.zoomLevel -= 0.5
                }
            }
        }

        // Boutons pour zoomer
        Column {
            anchors {
                right: parent.right
                bottom: parent.bottom
                margins: 10
            }

            Button {
                text: "+"
                onClicked: {
                    if (map.zoomLevel < 20) {
                        map.zoomLevel += 1
                    }
                }
            }

            Button {
                text: "-"
                onClicked: {
                    if (map.zoomLevel > 12) {
                        map.zoomLevel -= 1
                    }
                }
            }
        }
    }
}

