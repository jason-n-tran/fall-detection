import QtQuick
import QtQuick.Controls

Window {
    id: root
    width: 640
    height: 480
    visible: true
    title: qsTr("Sentinel Fall Detection")
    color: "#0f172a"

    property color accentColor: "#38bdf8"
    property bool isSafe: true

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: root.color }
            GradientStop { position: 1.0; color: Qt.darker(root.color, 1.2) }
        }
    }

    Connections {
        target: imuSensor

        function onStatusUpdated(status) {
            if (isSafe) {
                statusText.text = status;
            }
        }

        function onFallDetected() {
            root.color = "#450a0a";Night
            isSafe = false;
            titleText.text = "EMERGENCY: FALL DETECTED";
            statusText.text = "IMMEDIATE ASSISTANCE REQUIRED";
            fallIcon.text = "⚠️";
        }

        function onFallRiskWarning(risk) {
            if (isSafe) {
                riskAnim.start();
                riskWarningLabel.visible = true;
                riskResetTimer.restart();
            }
        }
    }

    Timer {
        id: riskResetTimer
        interval: 3000
        onTriggered: riskWarningLabel.visible = false
    }

    SequentialAnimation on color {
        id: riskAnim
        running: false
        onStarted: if (!isSafe) riskAnim.stop(); 
        ColorAnimation { to: "#422006"; duration: 200 }
        ColorAnimation { to: "#0f172a"; duration: 800 }
    }

    Item {
        id: visualizationContainer
        width: 300
        height: 300
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter

        Rectangle {
            id: deviceModel
            width: 120
            height: 200
            color: "#1e293b"
            radius: 10
            border.color: imuSensor.totalAccel > 1.5 ? "#f43f5e" : accentColor
            border.width: 2
            anchors.centerIn: parent
            
            Rectangle {
                width: 40; height: 10
                color: accentColor
                anchors.top: parent.top
                anchors.topMargin: 20
                anchors.horizontalCenter: parent.horizontalCenter
                radius: 5
                opacity: 0.8
            }

            transform: [
                Rotation { 
                    origin.x: deviceModel.width / 2
                    origin.y: deviceModel.height / 2
                    axis { x: 1; y: 0; z: 0 } 
                    angle: imuSensor.pitch * (180 / Math.PI)
                    Behavior on angle { NumberAnimation { duration: 20 } }
                },
                Rotation { 
                    origin.x: deviceModel.width / 2
                    origin.y: deviceModel.height / 2
                    axis { x: 0; y: 1; z: 0 } 
                    angle: imuSensor.roll * (180 / Math.PI)
                    Behavior on angle { NumberAnimation { duration: 20 } }
                },
                Rotation { 
                    origin.x: deviceModel.width / 2
                    origin.y: deviceModel.height / 2
                    axis { x: 0; y: 0; z: 1 } 
                    angle: imuSensor.yaw * (180 / Math.PI)
                    Behavior on angle { NumberAnimation { duration: 20 } }
                }
            ]

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: deviceModel.border.color
                border.width: 1
                opacity: 0.3
                radius: parent.radius
                scale: 1.05
            }
        }
    }

    Column {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 100
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 15
        
        Text {
            id: fallIcon
            text: "🛡️"
            font.pixelSize: 80
            anchors.horizontalCenter: parent.horizontalCenter
            
            SequentialAnimation on opacity {
                loops: Animation.Infinite
                NumberAnimation { from: 1.0; to: 0.5; duration: 2000; easing.type: Easing.InOutQuad }
                NumberAnimation { from: 0.5; to: 1.0; duration: 2000; easing.type: Easing.InOutQuad }
            }
        }

        Text {
            id: titleText
            text: "SYSTEM MONITORING"
            color: accentColor
            font.pixelSize: 18
            font.bold: true
            font.letterSpacing: 2
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            id: statusText
            font.pixelSize: 28
            color: "white"
            text: qsTr("Monitoring...")
            anchors.horizontalCenter: parent.horizontalCenter
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            width: root.width * 0.8
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10
            
            Rectangle {
                width: 200
                height: 8
                color: "#1e293b"
                radius: 4
                
                Rectangle {
                    width: Math.min(200, (imuSensor.totalAccel / 4.0) * 200)
                    height: parent.height
                    color: imuSensor.totalAccel > 2.0 ? "#ef4444" : "#10b981"
                    radius: 4
                    Behavior on width { NumberAnimation { duration: 50 } }
                }
            }
            
            Text {
                text: imuSensor.totalAccel.toFixed(2) + " G"
                color: "white"
                font.pixelSize: 12
                font.family: "Monospace"
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Rectangle {
            id: riskWarningLabel
            visible: false
            width: riskWarningText.width + 40
            height: 40
            color: "#422006"
            radius: 20
            border.color: "#f59e0b"
            border.width: 1
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                id: riskWarningText
                text: "⚠️ HIGH FALL RISK DETECTED"
                color: "#fbbf24"
                font.bold: true
                font.pixelSize: 14
                anchors.centerIn: parent
            }
        }
    }
    
    Button {
        id: resetButton
        text: "DISMISS ALERT"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        visible: !isSafe
        
        contentItem: Text {
            text: resetButton.text
            color: "white"
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            implicitWidth: 200
            implicitHeight: 50
            color: resetButton.pressed ? "#991b1b" : "#dc2626"
            radius: 25
        }
        
        onClicked: {
            root.color = "#0f172a";
            isSafe = true;
            titleText.text = "SYSTEM MONITORING";
            fallIcon.text = "🛡️";
            imuSensor.statusUpdated("Monitoring...");
        }
    }
}