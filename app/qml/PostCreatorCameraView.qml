
import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtMultimedia 5.4


ColumnLayout {
  id: root
  
  property alias hasImage: cameraView.hasImage
  property alias location: cameraView.location
  function clear() {
    locationField.hasImage = false
    locationField.location = ""
  }
  
  CameraView {
    id: cameraView
    Layout.fillWidth: true
    Layout.fillHeight: true
  }
}
