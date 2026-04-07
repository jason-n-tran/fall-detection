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
            statusText.text = status;
        }

        function onFallDetected() {
            root.color = "#450a0a";Night
            isSafe = false;
            titleText.text = "EMERGENCY: FALL DETECTED";
            statusText.text = "IMMEDIATE ASSISTANCE REQUIRED";
            fallIcon.text = "⚠️";
        }

        function onFallRiskWarning(risk) {
            riskAnim.start();
            riskWarningLabel.visible = true;
            riskResetTimer.restart();
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
        ColorAnimation { to: "#422006"; duration: 200 }
        ColorAnimation { to: "#0f172a"; duration: 800 }
    }

    Column {
        anchors.centerIn: parent
        spacing: 20
        
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
            font.pixelSize: 32
            color: "white"
            text: qsTr("Monitoring...")
            anchors.horizontalCenter: parent.horizontalCenter
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            width: root.width * 0.8
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