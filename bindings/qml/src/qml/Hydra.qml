
import QtQml 2.1

import QmlHydra 1.0


QtObject {
  id: root
  
  // Properties of the C hydra class
  property var nickname: undefined
  property string directory: "/tmp/hydra"
  property bool useLocalIPC: false
  property bool verbose: false
  property bool animate: false
  
  // Properties of the QML wrapper
  property int fetchInterval: 500 // how often to check for new messages (in ms)
  
  // This signal is emitted whenever a new post is fetched.
  // The post object is a static object containing the post's properties.
  signal fetched(var post)
  
  // Enter the given post object into hydra storage for sharing.
  signal store(var post)
  onStore: {
    if (post.location && post.location.length > 0)
      priv.hydra.storeFile(
        post.subject || "",
        post.parentId || "",
        post.mimeType || "",
        post.location)
    else
      priv.hydra.storeString(
        post.subject || "",
        post.parentId || "",
        post.mimeType || "text/plain",
        post.content || "")
  }
  
  // Creat and destroy the service along with this object
  Component.onCompleted: priv.createHydra()
  Component.onDestruction: priv.destroyHydra()
  
  // Allow QtObjects to be nested freely within (to no effect)
  property list<QtObject> children
  default property alias _children: root.children // Workaround; QTBUG-15127
  
  // Private implementation details
  property QtObject _priv: QtObject {
    id: priv
    
    // The underlying QmlHydra instance
    property QmlHydra hydra
    
    // Create a new QmlHydra instance with the current properties set
    function createHydra() {
      hydra = QmlHydra.construct(root.directory)
      
      if (!hydra.isNULL) {
        if (root.verbose)     hydra.setVerbose()
        if (root.animate)     hydra.setAnimate()
        if (root.useLocalIPC) hydra.setLocalIpc()
        if (root.nickname)    hydra.setNickname(root.nickname)
        
        hydra.start()
        fetcher.start()
      }
      else
        console.error("ERROR: hydra_new failed in directory:", root.directory)
    }
    
    // Destroy the current QmlHydra instance
    function destroyHydra() {
      fetcher.stop()
      QmlHydra.destruct(hydra)
    }
    
    // The Timer for periodically checking for new messages
    property Timer fetcher: Timer {
      interval: root.fetchInterval
      repeat: true
      triggeredOnStart: true
      
      // On each tick, fetch as many posts as are available from the service,
      // converting them to javascript objects and destroying the originals.
      onTriggered: {
        if (priv.hydra.isNULL) return
        var post
        while (!((post = priv.hydra.fetch()).isNULL)) {
          root.fetched ({
            parentId:    post.parentId(),
            ident:       post.ident(),
            subject:     post.subject(),
            content:     post.content(),
            timestamp:   post.timestamp(),
            // TODO: copy other properties
          })
          QmlHydraPost.destruct(post)
        }
      }
    }
  }
}
