
import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1

import QmlHydra 1.0


Rectangle {
  id: globalRoot
  width: 640
  height: 480
  
  Hydra {
    id: globalHydra
    directory: (qmlContextHomeDirectory || "/tmp") + "/hydra"
  }
  
  SystemPalette { id: colors; colorGroup: SystemPalette.Inactive }
  color: colors.window
  
  property MenuBar mainMenu: MenuBar {
    Menu {
      title: "&File"
      MenuItem {
        text: "&Create a post"
        onTriggered: postCreator.visible = true
      }
      MenuItem {
        // property alias verbose: globalHydra.verbose
        text: "&Verbose logging"
        checkable: true
        onCheckedChanged: globalHydra.verbose = checked
        checked: globalHydra.verbose
      }
      MenuItem {
        // property alias animate: globalHydra.animate
        text: "&Verbose discovery"
        checkable: true
        onCheckedChanged: globalHydra.animate = checked
        checked: globalHydra.animate
      }
      MenuItem {
        text: "E&xit"
        onTriggered: Qt.quit()
      }
    }
  }
  
  GridLayout {
    id: layout
    anchors.fill: parent
    
    property bool landscape: width >= height
    property bool small: (width+height) < 1500 // arbitrarily chosen for phones
    
    rows:    landscape ? 1 : 2
    columns: landscape ? 2 : 1
    
    PostCreator {
      id: postCreator
      Layout.row: 1
      Layout.column: layout.landscape ? 2 : 1
      Layout.rowSpan: 1
      Layout.columnSpan: 1
      Layout.fillWidth: true
      Layout.fillHeight: true
      clip: true
    }
    PostStream {
      id: postStream
      Layout.row: layout.landscape ? 1 : 2
      Layout.column: 1
      Layout.rowSpan: 1
      Layout.columnSpan: 1
      Layout.fillWidth: true
      Layout.fillHeight: true
      clip: true
      // Hide on small devices when postCreator is visible
      visible: !(layout.small && postCreator.visible)
    }
  }
}
