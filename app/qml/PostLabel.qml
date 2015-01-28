
import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1


Label {
  Layout.maximumWidth: parent.width
  Layout.fillWidth: true
  visible: text.length > 0
  // style: Text.Raised
  // styleColor: "#662233"
  horizontalAlignment: Text.AlignHCenter
  verticalAlignment: Text.AlignVCenter
  wrapMode: Text.Wrap
  
  property Menu menu
  
  // Popup the given context menu on right click or long press
  MouseArea {
    id: mouseArea
    anchors.fill: parent
    acceptedButtons: Qt.LeftButton | Qt.RightButton
    
    onPressAndHold:
      if(menu)
        menu.popup()
    onClicked:
      if(menu && mouse.button == Qt.RightButton)
        menu.popup()
  }
}
