
import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1


Item {
  id: root
  
  property alias subject: subjectField.text
  property alias content: textContentField.text
  property alias location: imageContentBrowser.text
  property string parentId: ""
  property string parentSubject: ""
  
  
  function storePost() {
    // TODO: implement
  }
  
  function clear() {
    subject = ""
    content = ""
    location = ""
    parentId = ""
    parentSubject = ""
    imageContentBrowser.clear()
  }
  
  GroupBox {
    width: parent.width * 0.9
    height: parent.height
    anchors.centerIn: parent
    
    GridLayout {
      Layout.maximumWidth: parent.width
      anchors.fill: parent
      columns: 1
      
      PostLabel { id: dummyLabel }
      
      PostLabel {
        text: "Create a new post"
        font.pointSize: dummyLabel.font.pointSize * 1.25
        font.weight: Font.Bold
      }
      
      PostLabel {
        text: "Parent Post: \"%1\"".arg(root.parentSubject)
        font.pointSize: dummyLabel.font.pointSize * 1.125
        font.weight: Font.Bold
        visible: root.parentId.length > 0
      }
      
      TextField {
        id: subjectField
        Layout.fillWidth: true
        placeholderText: "Subject goes here..."
        font.pointSize: dummyLabel.font.pointSize * 1.25
      }
      
      GridLayout {
        ExclusiveGroup { id: typeGroup }
        RadioButton {
          id: typePlainText
          text: "Plain Text"
          exclusiveGroup: typeGroup
          checked: true // Default
        }
        RadioButton {
          id: typeImageFile
          text: "Image File"
          exclusiveGroup: typeGroup
        }
        RadioButton {
          id: typeCameraImage
          text: "Camera Image"
          exclusiveGroup: typeGroup
        }
      }
      
      TextArea {
        id: textContentField
        Layout.fillWidth: true
        Layout.fillHeight: true
        text: "Content goes here..."
        visible: typePlainText.checked
      }
      
      PostCreatorImageBrowser {
        id: imageContentBrowser
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: typeImageFile.checked
      }
      
      Item {
        id: spacerItem // Grows to fill the vacuum when controls change
        Layout.fillWidth: true
        Layout.fillHeight: true
      }
      
      RowLayout {
        Button {
          id: sendButton
          Layout.fillWidth: true
          text: "Send"
          onClicked: { root.storePost(); root.clear(); root.visible = false }
        }
        Button {
          id: cancelButton
          Layout.fillWidth: true
          text: "Cancel"
          onClicked: { root.clear(); root.visible = false }
        }
      }
    }
  }
}
