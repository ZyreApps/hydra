
import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1


Rectangle {
  width: 500
  height: 500
  
  SystemPalette { id: colors; colorGroup: SystemPalette.Inactive }
  color: colors.window
  
  GridLayout {
    id: layout
    anchors.fill: parent
    
    property bool landscape: Screen.primaryOrientation==Qt.LandscapeOrientation
    rows:    landscape ? 1 : 2
    columns: landscape ? 2 : 1
    
    PostCreator {
      Layout.row: 1
      Layout.column: layout.landscape ? 2 : 1
      Layout.rowSpan: 1
      Layout.columnSpan: 1
      Layout.fillWidth: true
      Layout.fillHeight: true
      clip: true
    }
    PostStream {
      Layout.row: layout.landscape ? 1 : 2
      Layout.column: 1
      Layout.rowSpan: 1
      Layout.columnSpan: 1
      Layout.fillWidth: true
      Layout.fillHeight: true
      clip: true
    }
  }
}
