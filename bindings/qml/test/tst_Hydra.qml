
import QtTest 1.0
import QtQuick 2.1

import QmlHydra 1.0


TestCase {
  id: test
  name: "Hydra"
  
  Hydra {
    id: subject
  }
  
  function test_store_content() {
    verify(subject)
    console.log("starting")
    wait(1000)
    subject.store({
      subject: "A String Post",
      parentId: "",
      mimeType: "text/plain",
      content: "This is the content of the string post..."
    })
    wait(10000)
    console.log("stopping")
  }
}

