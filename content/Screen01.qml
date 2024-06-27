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
    property bool isNotifyBoxVisible: true
    property string notifyTextString: "Initialization"
    property string currentJob: ""
    property string currentPhase: ""
    property string currentTask: ""
    property bool currentCompletion: false



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

        property int notificationState: 0;
        property bool isInputTextVisible: false;

        Text {
            id: notifyText
            height: 50
            text: rectangle.notifyTextString
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 50
            anchors.rightMargin: 50
            anchors.topMargin: 10
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }//Text

        TextEdit {
            id: inputText
            anchors.top: notifyText.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.leftMargin: 10
            anchors.topMargin: 10
            text: ""
            visible: notifyBox.isInputTextVisible
        }

        Button {
            id: okButton
            //x: 122
            //y: 145
            //width: 93
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 80
            anchors.rightMargin: 80
            height: 50
            text: qsTr("OK")

            Connections {
                target: okButton
                onClicked: {
                    //Strip out all the references to UI properties here when setting the text.
                    //Access the properties through C++ functions to get reliable data.
                    //The asynchronous execution of signals is causing problems here.
                    var str
                    var cj = tsmodel.getCurrentJob()
                    var ct = tsmodel.getCurrentTask()
                    switch (notifyBox.notificationState) {
                    case 0:
                        str = tsmodel.getInitJobPath();
                        if (tsmodel.changeJob(str)){
                            if(tsmodel.getIsJobComplete(0)) {
                                notifyBox.notificationState =  4;
                                rectangle.notifyTextString = "Job: " + cj + " Complete"
                            } else {
                                notifyBox.notificationState = 3;
                                rectangle.isNotifyBoxVisible = false;
                            }
                        } else {
                            notifyBox.notificationState = 7
                            rectangle.notifyTextString = "No User Data Found"
                        }

                        break;
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
                    case 7:
                        rectangle.notifyTextString = "Enter a new job #"
                        //Make text field visible
                        notifyBox.notificationState = 5
                        notifyBox.isInputTextVisible = true;
                        break;
                    case 5:
                        let re = /\d{2}-\d{4}/
                        str = inputText.text
                        if (!re.test(str)) {
                            rectangle.notifyTextString = "Invalid input, please follow the job number format ##-####"
                            break;
                        }
                        if (tsmodel.changeJob(str)) {
                            tsmodel.changeTask()
                            rectangle.notifyTextString = "Starting Task: " + tsmodel.getCurrentTask()
                            notifyBox.notificationState = 2
                        } else {
                            rectangle.notifyTextString = "Job # Invalid,\nMaking Job..."
                            tsmodel.createJob(str)
                            tsmodel.changeJob(str)
                            tsmodel.changeTask()
                            notifyBox.notificationState = 6
                        }
                        inputText.text = ""
                        notifyBox.isInputTextVisible = false
                        break;
                    case 6:
                        notifyBox.notificationState=2
                        rectangle.notifyTextString = "Starting Task: " + tsmodel.getCurrentTask()
                        break;
                    }//switch
                }//onClicked
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
                    }//onClicked
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
                    }//onClicked
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
