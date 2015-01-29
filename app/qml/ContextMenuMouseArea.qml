
import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1


// Popup the given context menu on right click or long press
MouseArea {
  anchors.fill: parent
  acceptedButtons: Qt.LeftButton | Qt.RightButton
  
  property Menu menu
  
  onPressAndHold:
    if(menu)
      menu.popup()
  onClicked:
    if(menu && mouse.button == Qt.RightButton)
      menu.popup()
}
