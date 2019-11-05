import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Item {
    enabled: true
    smooth: true
    antialiasing: true
    clip: false

    width: childrenRect.width + 20

    FileDialog {
        id: fileDialogSaveRendering

        title: qsTr("Save rendering")
        selectFolder: false
        selectMultiple: false
        nameFilters: ["UEngine rendering files (*.ur)", "All files (*)"]
        selectExisting: false

        onAccepted: {
            var path = fileUrl.toString();
            path = path.replace(/^(file:\/{2})/,"");
            path = decodeURIComponent(path);

            app_manager.saveRendering(path);
        }
    }

    FileDialog {
        id: fileDialogSaveImage

        title: qsTr("Save image")
        selectFolder: false
        selectMultiple: false
        nameFilters: ["Image files (*.jpg *.png)"]
        selectExisting: false

        onAccepted: {
            var path = fileUrl.toString();
            path = path.replace(/^(file:\/{2})/,"");
            path = decodeURIComponent(path);

            app_manager.saveImage(path);
        }
    }

    GroupBox {
        id: groupBoxParameters
        x: 20
        y: 20

        title: qsTr("Rendering parameters")

        GridLayout {
            columns: 2

            Label {
                text: qsTr("Renderer Type:")
            }

            Label {
                id: labelRenderedType
                text: app_manager.rendererType
            }

            Label {
                text: qsTr("Image width:")
            }

            Label {
                id: labelImgWidth
                text: app_manager.imgWidth
            }

            Label {
                text: qsTr("Image height:")
            }

            Label {
                id: labelImgHeight
                text: app_manager.imgHeight
            }

            Label {
                text: qsTr("Pixel subdivison:")
            }

            Label {
                id: labelPixelSubdiv
                text: app_manager.pixelSubdiv
            }

            Label {
                text: qsTr("Lens subdivision:")
            }

            Label {
                id: labelLensSubdiv
                text: app_manager.lensSubdiv
            }

            Label {
                text: qsTr("Lens radius:")
            }

            Label {
                id: labelLensSize
                text: app_manager.lensSize
            }

            Label {
                text: qsTr("Focus plane distane:")
            }

            Label {
                id: labelFocusPlaneDistance
                text: app_manager.focusPlane
            }

            Label {
                text: qsTr("Min. depth:")
            }

            Label {
                id: labelMinDepth
                text: app_manager.minDepth
            }
        }
    }

    GroupBox {
        id: groupBoxStats
        y: 20

        anchors.left: groupBoxParameters.right
        anchors.leftMargin: 20

        title: qsTr("Statistics")

        GridLayout {
            rows: 2
            columns: 2

            Label {
                text: qsTr("Passes:")
            }

            Label {
                id: labelNumPasses
                text: app_manager.currPass
            }

            Label {
                text: qsTr("Avg. pass time:")
            }

            Label {
                id: labelAvgPassTime
                text: app_manager.avgPassTime
            }

            Label {
                text: qsTr("Threads:")
            }

            Label {
                id: labelNumThreads
                text: app_manager.numThreads
            }
        }
    }

    GroupBox {
        id: groupBoxStatus
        title: qsTr("Status")

        anchors.left: groupBoxStats.left
        anchors.top: groupBoxStats.bottom
        anchors.topMargin: 20

        GridLayout {
            rows: 2
            columns: 3

            Label {
                text: qsTr("Status:")
            }

            Label {
                text: app_manager.status
            }

            BusyIndicator {
                running: app_manager.running
            }

            Label {
                text: qsTr("Progress:")
            }

            Label {
                text: app_manager.progress
            }
        }
    }

    GroupBox {
        id: groupBoxImageConversion

        x: 20

        anchors.top: groupBoxParameters.bottom
        anchors.topMargin: 20

        title: qsTr("Image conversion")

        GridLayout {
            rows: 2
            columns: 3

            Label {
                text: qsTr("Color space")
            }

            Label {
                text: qsTr("Gamma")
            }

            Label {
                id: labelGammaValue
            }

            ComboBox {
                model: app_manager.rgbFormats

                onCurrentTextChanged: {
                    app_manager.setRgbFormat(currentText);
                }
            }

            Slider {
                id: slider

                Layout.columnSpan: 2

                stepSize: 0.1
                value: app_manager.gamma
                from: 0.1
                to: 5

                onValueChanged: {
                    labelGammaValue.text = slider.value.toFixed(1).toString();
                    app_manager.setGamma(value.toFixed(1))
                }
            }
        }
    }

    RowLayout {

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: groupBoxImageConversion.bottom
        anchors.margins: 20

        Button {
            id: buttonSaveRendering
            text: qsTr("Save Rendering")

            onClicked: {
                fileDialogSaveRendering.open();
            }
        }

        Button {
            id: buttonSaveImage
            text: qsTr("Save Image")

            onClicked: {
                fileDialogSaveImage.open();
            }
        }

        Button {
            id: buttonPresent
            text: qsTr("Present")

            onClicked: {
                var component = Qt.createComponent("PresentWindow.qml");
                var window    = component.createObject(root);
                window.show();
            }
        }
    }



}
