/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick 6.2
import QtQuick.Controls 6.2
import TaskSchedulerQtQuick
import QtQuick.Layouts

Rectangle {
    id: rectangle
    width: 400
    height: 600
    color: "#cfcfcf"
    property bool isNotifyBoxVisible: false
    property string notifyTextString: "Testing, testing"
    property string currentJob: ""
    property string currentPhase: ""
    property string currentTask: ""
    property bool currentCompletion: false
    property string previousJob: ""
    property string previousPhase: ""
    property string previousTask: ""
    property bool previousCompletion: false


    Rectangle {
        id: statusBox
        color: "#ffffff"
        radius: 5
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 20
        anchors.leftMargin: 20
        anchors.topMargin: 20
        height: 126
        visible: !rectangle.isNotifyBoxVisible

        ListView {
            id: listView
            anchors.fill: parent
            model: tsmodel
            delegate: currentTaskDelegate

            //property var item: model.get(0)
        }//ListView

        //Because this is a delegate to a list, it's locally scoped to that list.
        //Think of this as a struct definition that the ListView is instantiating.
        Component {
            id: currentTaskDelegate
            Item  {
                id: taskContainer
                anchors.fill: parent

                Text {
                    id: jobText
                    x: 8
                    y: 8
                    width: 77
                    height: 29
                    text: "Job: " +  model.job
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                }// Text

                Text {
                    id: taskText
                    x: 8
                    y: 78
                    width: 77
                    height: 29
                    text: "Task: " + model.name
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                }// Text

                Text {

                    id: phaseText
                    x: 8
                    y: 43
                    width: 77
                    height: 29
                    text: "Phase: " + model.phase
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                }//Text

                //I need a property reference to both the current and previous Task struct data.
                //This will be used in the chain of notification boxes.
                //The delay between responding to notification screens should be immensely slower than User Input, so this should be fine.
                Component.onCompleted: {
                    rectangle.previousJob = currentJob
                    rectangle.previousPhase = currentPhase
                    rectangle.previousTask = currentTask
                    rectangle.currentJob = model.job
                    rectangle.currentPhase = model.phase
                    rectangle.currentTask = model.name
                }
            }//Item
        }//Component
    }//Rectangle

    Rectangle {
        id: notifyBox
        height: 200
        color: "#ffffff"
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        visible: rectangle.isNotifyBoxVisible

        property int notificationState: 3;

        Text {
            id: notifyText
            x: 170
            height: 50
            text: rectangle.notifyTextString
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 50
            anchors.rightMargin: 50
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }//Text

        Button {
            id: okButton
            x: 122
            y: 145
            width: 93
            height: 47
            text: qsTr("OK")

            Connections {
                target: okButton
                onClicked: {
                    //Strip out all the references to UI properties here when setting the text.
                    //Access the properties through C++ functions to get reliable data.
                    //The asynchronous execution of signals is causing problems here.
                    var cj = tsmodel.getCurrentJob()
                    var ct = tsmodel.getCurrentTask()
                    switch (notifyBox.notificationState) {
                    case 1:
                        if(tsmodel.getIsJobComplete(0)) {
                            notifyBox.notificationState =  4;
                            rectangle.notifyTextString = "Job: " + cj + " Complete"
                        } else {
                            notifyBox.notificationState = 2;
                            rectangle.notifyTextString = "Starting Task: " + ct
                        }
                        break;
                    case 2:
                        rectangle.notifyTextString = ""
                        rectangle.isNotifyBoxVisible = false
                        break;
                    case 3:
                        console.log("How did you accomplish this?")
                        break;
                    case 4:
                        rectangle.notifyTextString = "Enter a new job #"
                        //Make text field visible
                        notifyBox.notificationState = 5
                        break;
                    case 5:
                        //Check if new changeJob() goes through. Probably going to have an async problem here since I access data between the change and some property bindings.
                        if (tsmodel.changeJob()) {
                            rectangle.notifyTextString = "Starting Task: " + ct
                            notifyBox.notificationState = 2
                        } else {
                            rectangle.notifyTextString = "Job # Invalid,\nTry Again"
                            notifyBox.notificationState = 6
                        }
                        break;
                    case 6: //I think I can condense state 4 and 5. I just enter the same state with a different notifyTextString.
                        rectangle.notifyTextString = "Enter a new job #"
                        //Make text field visible
                        notifyBox.notificationState=5
                        break;
                    }
                }
            }//Connections
        }//Button
    }//Rectangle

    Rectangle {
        id: taskButtonBox
        y: 695
        height: 60
        color: "#ffffff"
        radius: 5
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 20
        anchors.leftMargin: 20
        anchors.bottomMargin: 20
        visible: !rectangle.isNotifyBoxVisible

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 30
            anchors.rightMargin: 5
            anchors.topMargin: 5
            anchors.bottomMargin: 5
            Button {
                id: changeTaskButton
                text: qsTr("Change Task")

                Connections {
                    target: changeTaskButton
                    onClicked: {
                        tsmodel.changeTask();
                        rectangle.isNotifyBoxVisible = true
                        notifyBox.notificationState = 2
                        rectangle.notifyTextString = "Starting Task: " + tsmodel.getCurrentTask()
                    }
                }//Connections
            }//Button

            Button {
                id: completeTaskButton
                text: qsTr("Complete Task")

                Connections {
                    target: completeTaskButton
                    onClicked: {
                        notifyBox.notificationState = 1
                        rectangle.notifyTextString = "Task '" + tsmodel.getCurrentTask() + "' Completed.";
                        rectangle.isNotifyBoxVisible = true;
                        tsmodel.completeTask();
                        tsmodel.changeTask();
                    }
                }//Connections
            }//Button
        }//RowLayout
    }//Rectangle

    //This was generated from a button I made in design mode. It prints to qDebug() when I click the button it's attached to.
    states: [
        State {
            name: "clicked"
            when: changeTaskButton.checked
        }//State
    ]
}//Rectangle
