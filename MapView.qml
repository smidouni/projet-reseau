import QtQuick 2.15
import QtLocation 5.15
import QtPositioning 5.15

Rectangle {
    width: 800
    height: 600

    Plugin {
        id: mapPlugin
        name: "osm"
    }

    Map {
        anchors.fill: parent
        plugin: mapPlugin
        zoomLevel: initialZoomLevel  // Use initial zoom level from C++
        center: QtPositioning.coordinate(initialCenterLat, initialCenterLon)  // Use initial center from C++

        MapItemView {
            id: vehicleView
            model: simManager.vehiclesModel
            delegate: MapQuickItem {
                coordinate: QtPositioning.coordinate(modelData.lat, modelData.lon)
                anchorPoint.x: image.width / 2
                anchorPoint.y: image.height / 2
                sourceItem: Image {
                    id: image
                    source: "qrc:/images/car_icon.svg"
                    width: 32
                    height: 32
                }
            }
        }
    }
}
