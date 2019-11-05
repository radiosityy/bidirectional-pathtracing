import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Item {

    ImagePagePanel{
        id: panel

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
    }

    Image {
        id: image

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: panel.right

        anchors.margins: 20

        cache: false

        fillMode: Image.Pad
        horizontalAlignment: Image.AlignHCenter
        verticalAlignment: Image.AlignVCenter

        source: app_manager.previewImg

    }
}
