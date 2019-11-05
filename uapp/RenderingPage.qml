import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import QtQuick.Dialogs 1.2

Item {

    FileDialog{
        id: fileDialogLoadRendering

        title: qsTr("Load rendering")
        selectFolder: false
        selectMultiple: false
        nameFilters: ["UEngine rendering files (*.ur)", "All files (*)"]

        onAccepted: {
            var path = fileUrl.toString();
            path = path.replace(/^(file:\/{2})/,"");
            path = decodeURIComponent(path);

            app_manager.loadRendering(path);
        }
    }

    FileDialog{
        id: fileDialogLoadScene

        title: qsTr("Load scene")
        selectFolder: false
        selectMultiple: false
        nameFilters: ["Xml files (*.xml)", "All files (*)"]

        onAccepted: {
            var path = fileUrl.toString();
            path = path.replace(/^(file:\/{2})/,"");
            path = decodeURIComponent(path);

            app_manager.loadScene(path);
        }
    }

    GroupBox {
        id: groupBoxNewRendering
        x: 20
        y: 20
        title: qsTr("New rendering")

        GridLayout {
            columnSpacing: 15

            rows: 5
            columns: 4

            Label {
                text: qsTr("Image width")
            }

            RenderingPageTextField {
                id: textFieldImageWidth
                text: qsTr("1280")
            }

            Label {
                text: qsTr("Image height")
            }

            RenderingPageTextField {
                id: textFieldImageHeight
                text: qsTr("720")
            }

            Label {
                text: qsTr("Pixel subdivision")
            }

            RenderingPageTextField {
                id: textFieldPixelSubdiv
                text: qsTr("1")
            }

            Label {
                text: qsTr("Lens subdivision")
            }

            RenderingPageTextField {
                id: textFieldLensSubdiv
                text: qsTr("1")
            }

            Label {
                text: qsTr("Focus plane")
            }

            RenderingPageTextField {
                id: textFieldFoucsPlane
                text: qsTr("1.0")
            }

            Label {
                text: qsTr("Lens radius")
            }

            RenderingPageTextField {
                id: textFieldLensRadius
                text: qsTr("0.0001")
            }

            Label {
                text: qsTr("Min. depth")
            }

            RenderingPageTextField {
                id: textFieldMinDepth
                text: qsTr("5")
            }

            Label {
                text: qsTr("Renderer type")
                Layout.column: 0
                Layout.row: 4
            }

            ComboBox {
                id: comboBoxRendererType

                Layout.column: 1
                Layout.row: 4
                Layout.preferredWidth: 150

                model: app_manager.rendererTypes;
            }

            Button{
                id: buttonNewRendering
                text: qsTr("New rendering")

                Layout.column: 3
                Layout.row: 4

                onClicked: {
                    app_manager.newRendering([textFieldImageWidth.text,
                                              textFieldImageHeight.text,
                                              textFieldPixelSubdiv.text,
                                              textFieldLensSubdiv.text,
                                              textFieldLensRadius.text,
                                              textFieldFoucsPlane.text,
                                              textFieldMinDepth.text,
                                              comboBoxRendererType.currentText]);
                }
            }
        }
    }

    GroupBox {
        id: groupBoxLoadRendering
        title: qsTr("Load rendering")

        x: 20
        anchors.top: groupBoxNewRendering.bottom
        anchors.topMargin: 20

        Button {
            id: buttonLoadRendering
            text: qsTr("Load")
            width: 130

            onClicked: {
                fileDialogLoadRendering.open();
            }
        }
    }

    GroupBox {
        id: groupBoxScene
        title: qsTr("Scene")

        anchors.top: groupBoxNewRendering.bottom
        anchors.left: groupBoxLoadRendering.right
        anchors.topMargin: 20
        anchors.leftMargin: 20

        RowLayout{

        Button {
            id: buttonLoadScene
            text: qsTr("Load Scene")
//            width: 100

            onClicked: {
                fileDialogLoadScene.open();
            }
        }

        Button {
            id: buttonRefreshScene
            text: qsTr("Refresh Scene")
//            width: 100

            onClicked: {
                app_manager.refreshScene();
            }
        }
        }
    }

    GroupBox {
        id: groupBoxRendering
        title: qsTr("Rendering")

        x: 20
        anchors.top: groupBoxLoadRendering.bottom
        anchors.topMargin: 20

        GridLayout {

            rows: 2
            columns: 2

            Label{
                text: qsTr("Threads")
            }

            RenderingPageTextField {
                id: textFieldNumThreads
                text: qsTr("8")
            }

            Button {
                id: buttonStart
                text: qsTr("Start")

                Layout.preferredWidth: 150

                onClicked: {
                    app_manager.startRendering(textFieldNumThreads.text);
                }
            }

            Button {
                id: buttonStop
                text: qsTr("Stop")

                Layout.preferredWidth: 150

                onClicked: {
                    app_manager.stopRendering();
                }
            }
        }
    }

}
