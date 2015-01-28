
import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtMultimedia 5.4


ColumnLayout {
  id: root
  
  property bool hasImage: false
  property string location: ""
  property bool isFullscreen: false
  
  Camera {
    id: camera
    
    imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceAuto
    flash.mode: Camera.FlashAuto
    
    imageCapture {
      onImageCaptured: {
        imagePreview.source = preview
        largeImagePreview.source = preview
      }
      onImageSaved: {
        root.hasImage = true
        root.location = path
      }
    }
  }
  
  // Don't run the camera unless we are actively viewing its output
  Component.onCompleted: camera.stop()
  onIsFullscreenChanged: {
    if (isFullscreen)
      camera.start()
    else
      camera.stop()
  }
  
  ColumnLayout {
    id: imageContainer
    Layout.fillWidth: true
    Layout.fillHeight: true
    
    Image {
      id: imagePreview
      Layout.fillWidth: true
      Layout.fillHeight: true
      fillMode: Image.PreserveAspectFit
      visible: root.hasImage
    }
    
    RowLayout {
      Button {
        id: goToCaptureButton
        Layout.fillWidth: true
        text: "Go To Capture"
        onClicked: root.isFullscreen = true
      }
    }
  }
  
  Rectangle {
    id: largeImageContainerBackground
    parent: globalRoot
    anchors.fill: parent
    color: "#CC000000"
    visible: root.isFullscreen
    
    ColumnLayout {
      id: largeImageContainer
      anchors.fill: parent
      
      VideoOutput {
        id: largeImageLive
        Layout.fillWidth: true
        Layout.fillHeight: true
        source: camera
        focus: visible
        visible: !root.hasImage
        autoOrientation: true
      }
      
      Image {
        id: largeImagePreview
        Layout.fillWidth: true
        Layout.fillHeight: true
        fillMode: Image.PreserveAspectFit
        visible: root.hasImage
      }
      
      RowLayout {
        Button {
          id: captureButton
          Layout.fillWidth: true
          text: "Capture"
          onClicked: camera.imageCapture.capture()
          visible: !root.hasImage
        }
        Button {
          id: retryButton
          Layout.fillWidth: true
          text: "Retry"
          onClicked: root.hasImage = false
          visible: root.hasImage
        }
        Button {
          id: switcButton
          Layout.fillWidth: true
          text: "Switch"
          
          onClicked: { // Cycle through available cameras
            var list = QtMultimedia.availableCameras
            var index = 0
            for (var i in list)
              if (list[i].deviceId == camera.deviceId)
                index = parseInt(i)
            index += 1
            if (index >= list.length)
              index = 0
              console.log(index)
            camera.deviceId = list[index.toString()].deviceId
          }
          visible: QtMultimedia.availableCameras.length > 1
        }
        Button {
          id: doneButton
          Layout.fillWidth: true
          text: "Done"
          onClicked: root.isFullscreen = false
        }
      }
    }
  }
}
