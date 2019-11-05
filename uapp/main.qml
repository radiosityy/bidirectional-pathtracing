import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

ApplicationWindow {
    id: root

    visible: true
    width: 1024
    height: 720

    minimumWidth: 800
    minimumHeight: 720

    opacity: 1
    title: qsTr("UApp")

    onClosing: {
        app_manager.onClosing();
    }

    header : TabBar{
        id: tabBar

        currentIndex: swipeView.currentIndex

        Repeater{
            model: ["Rendering", "Image"]

            TabButton{
                text: modelData
            }
        }
    }

    SwipeView {
        id: swipeView
        currentIndex: tabBar.currentIndex

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: logScrollView.top

        RenderingPage{}
        ImagePage{}
    }

    ScrollView{
        id: logScrollView

        background: Rectangle{color: "white"; border.width: 1}

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 20

        height: 0.2 * parent.height

        TextArea {
            id: logTextArea
            objectName: "logTextArea"
            text: app_manager.logText

            font.family: "Courier"
            textFormat: Text.AutoText
            font.pointSize: 10

            readOnly: true
            enabled: true
            smooth: true
            antialiasing: true

            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
            selectByMouse: true
        }

    }

}
