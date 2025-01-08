// MapView.qml
import QtQuick
import QtQuick.Controls 2.15
import QtLocation 5.15
import QtPositioning 5.15
import QtQuick.Shapes 1.8

Rectangle {
    width: 800
    height: 600

    // Plugin OpenStreetMap
    Plugin {
        id: mapPlugin
        name: "osm"
    }

    // Map widget
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
                coordinate: QtPositioning.coordinate(modelData.lat, modelData.lon)
                anchorPoint.x: communicationCircle.width / 2
                anchorPoint.y: communicationCircle.height / 2

                sourceItem: Item {
                    id: container
                    width: communicationCircle.width
                    height: communicationCircle.height

                    // Communication range circle
                    Rectangle {
                        id: communicationCircle
                        anchors.centerIn: parent
                        width: modelData.communicationRange / mapMetersPerPixel(map.zoomLevel) * 2
                        height: width
                        radius: width / 2
                        visible: true
                        opacity: 0.03
                        gradient: RadialGradient {
                            centerX: 0.5
                            centerY: 0.5
                            centerRadius: 0.5 // Radius relative to the rectangle size
                            stops: [
                                GradientStop { position: 0.0; color: modelData.color; },
                                GradientStop { position: 1.0; color: "transparent"; }
                            ]
                        }
                    }

                    Image {
                        id: carIcon
                        anchors.centerIn: parent
                        source: modelData.messageReceived ? "qrc:/images/car_green.svg" : "qrc:/images/car_icon.svg"
                        width: 32
                        height: 32
                    }
                }
            }
        }

        MapItemView {
            id: communicationLinksView
            model: simManager.communicationLinksModel

            delegate: MapPolyline {
                line.color: "green"
                line.width: 2
                path: [
                    QtPositioning.coordinate(startLat, startLon),
                    QtPositioning.coordinate(endLat, endLon)
                ]
            }
        }

        // Render Blocked Edges as Red Lines using BlockedEdgesModel
        MapItemView {
            anchors.fill: parent
            model: blockedEdgesModel

            delegate: MapPolyline {
                //line.color: flashing ? "green" : "red"
                line.color: "red"
                line.width: 3
                path: [
                    QtPositioning.coordinate(startLat, startLon),
                    QtPositioning.coordinate(endLat, endLon)
                ]

                Component.onCompleted: {
                    console.log("ListView MapPolyline created with path:", path);
                }
            }
        }

        // Interaction zone for zooming and dragging
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            // Drag to move the map
            property var lastMousePosition
            onPressed: lastMousePosition = Qt.point(mouse.x, mouse.y)
            onPositionChanged: {
                if (mouse.buttons & Qt.LeftButton) {
                    var dx = lastMousePosition.x - mouse.x
                    var dy = lastMousePosition.y - mouse.y
                    lastMousePosition = Qt.point(mouse.x, mouse.y)

                    var metersPerPixel = mapMetersPerPixel(map.zoomLevel)
                    var newCenter = QtPositioning.coordinate(
                        map.center.latitude - (dy * metersPerPixel / 111320),
                        map.center.longitude + (dx * metersPerPixel / (111320 * Math.cos(map.center.latitude * Math.PI / 180)))
                    )
                    map.center = newCenter
                }
            }

            // Zoom with the mouse wheel
            onWheel: {
                if (wheel.angleDelta.y > 0 && map.zoomLevel < 20) {
                    map.zoomLevel += 0.5
                } else if (wheel.angleDelta.y < 0 && map.zoomLevel > 12) {
                    map.zoomLevel -= 0.5
                }
            }
        }

        // Zoom buttons
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

    // Utility function to calculate meters per pixel
    function mapMetersPerPixel(zoomLevel) {
        const earthCircumferenceMeters = 40075016.686;
        const tileSizePixels = 256;
        return earthCircumferenceMeters / (tileSizePixels * Math.pow(2, zoomLevel));
    }
}
