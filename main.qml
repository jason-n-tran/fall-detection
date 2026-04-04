import QtQuick
import QtQuick.Controls

Window {
    id: root
    width: 640
    height: 480
    visible: true
    title: qsTr("Fall Detector")
    color: "black"

    Connections {
        target: imuSensor

        function onStatusUpdated(status) {
            statusText.text = status;
        }

        function onFallDetected() {
            root.color = "#a01c1c";
            resetButton.visible = true;
        }
    }

    Text {
        id: statusText
        anchors.centerIn: parent
        font.pixelSize: 40
        color: "white"
        text: qsTr("Initializing...")
    }
    
    Button {
        id: resetButton
        text: "Reset Alert"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        visible: false
        
        onClicked: {
            root.color = "black";background color
            imuSensor.statusUpdated("Monitoring...");
            resetButton.visible = false;
        }
    }
}