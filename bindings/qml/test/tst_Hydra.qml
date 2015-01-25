
import QtTest 1.0
import QtQuick 2.1

import QmlHydra 1.0


TestCase {
  id: test
  name: "Hydra"
  
  Hydra {
    id: subject
  }
  
  function test_it() {
    verify(subject)
    console.log("starting")
    wait(10000)
    console.log("stopping")
  }
}

