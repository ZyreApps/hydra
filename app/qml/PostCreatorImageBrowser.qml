
import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1


ColumnLayout {
  id: root
  
  property alias location: locationField.text
  function clear() {
    locationField.text = ""
    locationField.accepted()
  }
  
  FileDialog { // This file dialog is currently very ugly on Android...
    id: fileDialog
    title: "Select an image file..."
    folder: "file:///sdcard"
    onFileUrlChanged: {
      locationField.text = fileUrl.toString().replace(/^(file:\/\/)/,"")
      locationField.accepted()
    }
  }
  RowLayout {
    Layout.fillWidth: true
    Layout.fillHeight: true
    Button {
      id: imageContentBrowseButton
      Layout.fillWidth: true
      text: "Browse"
      onClicked: fileDialog.open()
    }
    TextField {
      id: locationField
      Layout.fillWidth: true
      placeholderText: "/path/to/the/image..."
      onAccepted: imagePreview.source = text
    }
  }
  Image {
    id: imagePreview
    Layout.fillWidth: true
    Layout.fillHeight: true
    fillMode: Image.PreserveAspectFit
    
    MouseArea {
      anchors.fill: parent
      onClicked: imageLargePreview.visible = true
      enabled: imagePreview.status == Image.Ready
    }
  }
  Rectangle {
    id: imageLargePreview
    parent: globalRoot
    anchors.fill: parent
    color: "#AA000000"
    visible: false
    
    Image {
      anchors.fill: parent
      fillMode: Image.PreserveAspectFit
      source: imagePreview.source
    }
    MouseArea {
      anchors.fill: parent
      onClicked: imageLargePreview.visible = false
    }
  }
}
