
import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1


ListView {
  model: 9
  delegate: Post {
    width: parent.width * 0.9
    anchors.horizontalCenter: parent.horizontalCenter
  }
}
