import QtQuick 2.15
import QtLocation 5.15
import QtPositioning 5.15

Rectangle {
    width: 800
    height: 600

    // Déclaration du plugin OpenStreetMap
    Plugin {
        id: osmPlugin  // Ajoutez un ID pour le plugin
        name: "osm"
        PluginParameter { name: "osm.useragent"; value: "My great Qt OSM application" }
        PluginParameter { name: "osm.mapping.host"; value: "http://osm.tile.server.address/" }
        PluginParameter { name: "osm.mapping.copyright"; value: "All mine" }
        PluginParameter { name: "osm.routing.host"; value: "http://osrm.server.address/viaroute" }
        PluginParameter { name: "osm.geocoding.host"; value: "http://geocoding.server.address" }
    }

    Map {
        anchors.fill: parent
        plugin: osmPlugin  // Utilisation de l'ID pour référencer le plugin
        center: QtPositioning.coordinate(47.7508, 7.3359) // Coordonnées de Mulhouse
        zoomLevel: 14 // Niveau de zoom initial

        // Ajout d'un marqueur pour Mulhouse
        MapQuickItem {
            coordinate: QtPositioning.coordinate(47.7508, 7.3359)
            anchorPoint.x: marker.width / 2
            anchorPoint.y: marker.height
            sourceItem: Image {
                id: marker
                source: "qrc:/images/marker.png" // Assurez-vous que ce chemin est correct
                width: 32
                height: 32
            }
        }
    }
}
