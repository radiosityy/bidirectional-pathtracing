import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

ApplicationWindow {

    width: app_manager.imgWidth
    height: app_manager.imgHeight
    minimumWidth: app_manager.imgWidth
    minimumHeight: app_manager.imgHeight

    Image{
        cache: false

        fillMode: Image.Pad
        horizontalAlignment: Image.AlignHCenter
        verticalAlignment: Image.AlignVCenter

        anchors.fill: parent
        source: "image://app_image_provider/present_img"
    }
}
