
import QtQml 2.1

import QmlHydra 1.0


QtObject {
  id: root
  
  // Properties of the C hydra class
  property var nickname: undefined
  property var hostname: undefined
  property string directory: "/tmp/hydra"
  property bool useLocalIPC: false
  property bool verbose: false
  property bool animate: false
  
  // Restart the hydra service when any one of these properties changes
  onNicknameChanged:    priv.restartHydra()
  onHostnameChanged:    priv.restartHydra()
  onDirectoryChanged: { priv.restartHydra(); priv.fetchInitialFromLedger() }
  onUseLocalIPCChanged: priv.restartHydra()
  onVerboseChanged:     priv.restartHydra()
  onAnimateChanged:     priv.restartHydra()
  
  // Properties of the QML wrapper
  property int fetchInterval: 500 // how often to check for new messages (in ms)
  
  // This signal is emitted whenever a new post is fetched.
  // The post object is a static object containing the post's properties.
  signal fetched(var post)
  
  // Enter the given post object into hydra storage for sharing.
  signal store(var post)
  onStore: priv.store(post)
  
  // Creat and destroy the service along with this object
  Component.onCompleted: { priv.createHydra(); priv.fetchInitialFromLedger() }
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
        if (root.hostname)    hydra.setHostname(root.hostname)
        
        hydra.start()            // Start the underlying hydra actors
        fetcher.start()          // Start fetching posts from other nodes
      }
      else
        console.error("ERROR: hydra_new failed in directory:", root.directory)
    }
    
    // Destroy the current QmlHydra instance
    function destroyHydra() {
      fetcher.stop()
      QmlHydra.destruct(hydra)
    }
    
    // Restart the QmlHydra instance
    function restartHydra() { if (hydra) { destroyHydra(); createHydra() } }
    
    // Grab existing posts from our node's store
    function fetchInitialFromLedger() {
      var ledger = QmlHydraLedger.construct()
      ledger.load()
      var post
      var i = 0
      while (!((post = ledger.fetch(i++)).isNULL))
        handleFetched(post)
      
      QmlHydraLedger.destruct(ledger)
    }
    
    // Store a new post in our node's store
    function store(post) {
      if (!hydra || hydra.isNULL) return
      post.subject  = post.subject  || ""
      post.parentId = post.parentId || ""
      post.mimeType = post.mimeType || ""
      post.content  = post.content  || ""
      post.location = post.location || ""
      
      if (post.location.length > 0)
        hydra.storeFile(
          post.subject, post.parentId, post.mimeType, post.location)
      else
        hydra.storeString(
          post.subject, post.parentId, post.mimeType, post.content)
      
      echoStored(post)
    }
    
    // Echo posts from 'store' back to self to simulate fetch for local viewing
    function echoStored(post) {
      var echo = QmlHydraPost.construct(post.subject)
      echo.setParentId(post.parentId)
      echo.setMimeType(post.mimeType)
      
      if (post.location.length > 0)
        echo.setFile(post.location)
      else
        echo.setContent(post.content)
      
      handleFetched(echo)
    }
    
    // Translate a C post object into a JS obect and send in a fetched signal
    function handleFetched(post) {
      root.fetched ({
        parentId:    post.parentId(),
        ident:       post.ident(),
        timestamp:   post.timestamp(),
        subject:     post.subject(),
        mimeType:    post.mimeType(),
        content:     post.content(),
        location:    post.location(),
      })
      QmlHydraPost.destruct(post)
    }
    
    // The Timer for periodically checking for new messages
    property Timer fetcher: Timer {
      interval: root.fetchInterval
      repeat: true
      triggeredOnStart: true
      
      // On each tick, fetch as many posts as are available from the service,
      // converting them to javascript objects and destroying the originals.
      onTriggered: {
        if (!priv.hydra || priv.hydra.isNULL) return
        var post
        while (!((post = priv.hydra.fetch()).isNULL))
          priv.handleFetched(post)
      }
    }
  }
}
