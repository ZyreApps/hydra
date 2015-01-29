
import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1


GroupBox {
  id: root
  
  property var post: model
  
  property string ident:      post.ident      || ""
  property string timestamp:  post.timestamp  || ""
  property string subject:    post.subject    || ""
  property string mimeType:   post.mimeType   || ""
  property string content:    post.content    || ""
  property string location:   post.location   || ""
  
  property var replies:  {
    var ary = []
    for (var i in post.childrenIds) {
      var reply = postList.findPost("ident", post.childrenIds[i])
      if (reply)
        ary.push(reply)
    }
    return ary
  }
  
  property bool isImage: root.mimeType.indexOf("image/") == 0
  
  ColumnLayout {
    id: layout
    width: root.width * 0.888888888888888
    anchors.horizontalCenter: parent.horizontalCenter
    spacing: 2
    
    PostLabel {
      id: subjectLabel
      text: root.subject
      menu: contextMenu
      font.pointSize: contentLabel.font.pointSize * 1.5
      font.weight: Font.Bold
    }
    
    PostLabel {
      id: timeLabel
      property var date: new Date(root.timestamp)
      text: date.toLocaleDateString() + " " + date.toLocaleTimeString()
      menu: contextMenu
      font.pointSize: contentLabel.font.pointSize * 0.75
      font.weight: Font.Bold
    }
    
    PostLabel {
      id: contentLabel
      text: root.content
      menu: contextMenu
    }
    
    Image {
      Layout.fillHeight: true
      Layout.fillWidth: true
      Layout.maximumWidth: parent.width
      Layout.maximumHeight: parent.width * 1.25
      Layout.preferredHeight: parent.width * sourceSize.height / sourceSize.width
      id: contentImage
      fillMode: Image.PreserveAspectFit
      source: visible ? "file://" + root.location : ""
      visible: root.isImage && root.location.length > 0
      ContextMenuMouseArea { menu: contextMenu }
    }
    
    CheckBox {
      id: showRepliesCheckBox
      Layout.alignment: Qt.AlignHCenter
      text: "Show %1 replies   ".arg(root.replies.length)
      visible: root.replies.length > 0
    }
    
    // This Repeater will embed the stream of reply posts for this post
    // (posts with this post as a parent) when the showRepliesCheckBox
    // above is checked.
    Repeater {
      model: showRepliesCheckBox.checked ? root.replies : []
      Loader {
        Layout.maximumWidth: layout.width
        Layout.fillWidth: true
        active: showRepliesCheckBox.checked
        source: "Post.qml"
        Component.onCompleted: item.post = root.replies[index]
      }
    }
  }
  
  Menu {
    id: contextMenu
    MenuItem {
      text: "&Create a new post"
      onTriggered: postCreator.visible = true
    }
    MenuItem {
      text: "&Reply to this post"
      onTriggered: {
        postCreator.parentSubject = root.subject
        postCreator.parentId = root.ident
        postCreator.visible = true
      }
    }
    MenuItem {
      text: "&Delete this post"
      enabled: false // TODO: Implement
    }
    MenuItem {
      property bool running: true
      text: running ? "&Stop sharing" : "&Start sharing"
      enabled: false // TODO: Implement
    }
  }
}
