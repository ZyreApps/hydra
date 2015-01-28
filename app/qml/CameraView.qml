
import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtMultimedia 5.4


ColumnLayout {
  id: root
  
  property bool hasImage: false
  property string location: ""
  
  Camera {
    id: camera
    
    imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceAuto
    flash.mode: Camera.FlashAuto
    
    imageCapture {
      onImageCaptured: {
        imagePreview.source = preview
      }
      onImageSaved: {
        root.hasImage = true
        root.location = path
      }
    }
  }
  
  ColumnLayout {
    id: imageContainer
    Layout.fillWidth: true
    Layout.fillHeight: true
    
    VideoOutput {
      id: imageLive
      Layout.fillWidth: true
      Layout.fillHeight: true
      source: camera
      focus: visible
      visible: !root.hasImage
      autoOrientation: true
    }
    
    Image {
      id: imagePreview
      Layout.fillWidth: true
      Layout.fillHeight: true
      fillMode: Image.PreserveAspectFit
      visible: root.hasImage
    }
  }
  
  RowLayout {
    Button {
      id: captureButton
      Layout.fillWidth: true
      Layout.fillHeight: true
      text: "Capture"
      onClicked: camera.imageCapture.capture()
      visible: !root.hasImage
    }
    Button {
      id: retryButton
      Layout.fillWidth: true
      Layout.fillHeight: true
      text: "Retry"
      onClicked: root.hasImage = false
      visible: root.hasImage
    }
  }
}
