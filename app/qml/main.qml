
import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2


ApplicationWindow {
  title: "Hydra"
  width: 640
  height: 480
  visible: true
  
  menuBar: mainContent.mainMenu
  
  MainContent {
    id: mainContent
    anchors.fill: parent
  }
}
