
import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1


GroupBox {
  id: root
  
  ///
  // TODO: get rid of this mock data and use real posts
  property var post: ({
    subject: "Welcome to Edgenet:Hydra",
    content: "The goal of the Hydra project is to explore and learn "+
      "how to share knowledge and information across short-lived wireless "+
      "networks between mobile users. Hydra exploits \"proximity networks\", "+
      "that is the physical closeness of individuals. The Hydra project is "+
      "part of the edgenet experiment to design and build decentralized "+
      "networks, with no dependency on broadband Internet nor "+
      "centralized services.",
    replies: [reply1, reply2, reply3]
  })
  
  property var reply1: ({
    content: "Plausibly, Hydra could evolve into a fully decentralized "+
      "social network, closely mapping and amplifying existing human "+
      "protocols for knowledge and information sharing. However at this "+
      "stage, Hydra is simply a model for exchanging cat photos.",
    replies: []
  })
  
  property var reply2: ({
    content: "Note that Hydra is not a mesh network and does no "+
      "forwarding or routing. This is deliberate. We assume that peers "+
      "move around too rapidly for routing knowledge to ever be valid. "+
      "All a peer can ever know is \"I can talk to this other peer, for now.\"",
    replies: []
  })
  
  property var reply3: ({
    content: "Hydra is aimed, in its current incarnation, at technical "+
      "conferences, weddings, parties, and funerals. The participants are "+
      "in rough proximity, for a period of hours or days. They share a "+
      "strong set of interests. They implicitly trust each other, yet do "+
      "not care about identities. They mainly want to share photos and "+
      "comments on photos.",
    replies: []
  })
  
  
  property string ident:    post.ident    || ""
  property string subject:  post.subject  || ""
  property string mimeType: post.mimeType || ""
  property string content:  post.content  || ""
  property string location: post.location || ""
  property var replies:     post.replies  || []
  
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
      id: contentLabel
      text: root.content
      menu: contextMenu
    }
    
    Image {
      Layout.fillHeight: true
      Layout.fillWidth: true
      Layout.maximumWidth: parent.width
      Layout.preferredHeight: parent.width * sourceSize.height / sourceSize.width
      id: contentImage
      fillMode: Image.PreserveAspectFit
      source: visible ? "file://" + root.location : ""
      visible: root.location.length > 0
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
