
import QtTest 1.0
import QtQuick 2.1

import QmlHydra 1.0
import "../qml"


TestCase {
  id: test
  name: "HydraPostList"
  
  HydraPostList {
    id: subject
  }
  
  property var examplePost:({
    parentId: "",
    ident: "8EFC0240F18B5D4F3C7871E53978C839304B2062",
    subject: "Subject Line",
    content: "This is an example post.",
    timestamp: "2015-01-23T17:05:17Z",
  })
  
  property var childPostA:({
    parentId: "8EFC0240F18B5D4F3C7871E53978C839304B2062",
    ident: "540C261DDFF56571FC2FD7DCFF5BFCA561B0C8BD",
    subject: "Re: Subject Line",
    content: "Posts can be children by giving a parentId.",
    timestamp: "2015-01-23T17:06:32Z",
  })
  
  property var childPostB:({
    parentId: "8EFC0240F18B5D4F3C7871E53978C839304B2062",
    ident: "58D26159DDAE9DE6DD9185924800720E90CF6A17",
    subject: "Re: Subject Line",
    content: "Child-post-to-parent-post is a many-to-one relationship.",
    timestamp: "2015-01-23T17:07:45Z",
  })
  
  // Clear list between each test
  function init() {
    subject.clear()
  }
  
  // Helper function to clone a "flat" object
  function cloneObject(obj) {
    var clone = {}
    for (var key in obj)
      clone[key] = obj[key]
    return clone
  }
  
  // Helper function to compare a "flat" object
  function compareObject(obj, expected) {
    verify(obj)
    for (var key in expected) {
      compare(obj[key], expected[key],
        "key '%1' in %2".arg(key).arg(JSON.stringify(obj)))
    }
  }
  
  // Helper function to compare an array of "flat" objects
  function compareObjectArray(ary, expected) {
    verify(ary)
    compare(ary.length, expected.length)
    for (var i=0; i < expected.length; ++i) {
      compareObject(ary[i], expected[i])
    }
  }
  
  
  function test_addPost() {
    subject.addPost(cloneObject(examplePost))
    compareObject(subject.get(0), examplePost)
  }
  
  function test_addPost_InsertsMostRecentTimestampFirst() {
    subject.addPost(cloneObject(childPostA))
    subject.addPost(cloneObject(examplePost)) // added in arbitrary order
    subject.addPost(cloneObject(childPostB))
    compareObject(subject.get(0), childPostB)
    compareObject(subject.get(1), childPostA)
    compareObject(subject.get(2), examplePost)
    subject.clear()
    subject.addPost(cloneObject(examplePost)) // added in arbitrary order
    subject.addPost(cloneObject(childPostB))
    subject.addPost(cloneObject(childPostA))
    compareObject(subject.get(0), childPostB)
    compareObject(subject.get(1), childPostA)
    compareObject(subject.get(2), examplePost)
    subject.clear()
    subject.addPost(cloneObject(childPostA))
    subject.addPost(cloneObject(childPostB))
    subject.addPost(cloneObject(examplePost)) // added in arbitrary order
    compareObject(subject.get(0), childPostB)
    compareObject(subject.get(1), childPostA)
    compareObject(subject.get(2), examplePost)
  }
  
  function test_addPost_SetsChildrenIdentsArray() {
    var parentPost
    subject.addPost(cloneObject(childPostA))
    subject.addPost(cloneObject(examplePost)) // added in arbitrary order
    subject.addPost(cloneObject(childPostB))
    parentPost = subject.findPost("ident", examplePost.ident)
    compare(parentPost.childrenIds[0], childPostB.ident)
    compare(parentPost.childrenIds[1], childPostA.ident)
    subject.clear()
    subject.addPost(cloneObject(examplePost)) // added in arbitrary order
    subject.addPost(cloneObject(childPostA))
    subject.addPost(cloneObject(childPostB))
    parentPost = subject.findPost("ident", examplePost.ident)
    compare(parentPost.childrenIds[0], childPostB.ident)
    compare(parentPost.childrenIds[1], childPostA.ident)
    subject.clear()
    subject.addPost(cloneObject(childPostA))
    subject.addPost(cloneObject(childPostB))
    subject.addPost(cloneObject(examplePost)) // added in arbitrary order
    parentPost = subject.findPost("ident", examplePost.ident)
    compare(parentPost.childrenIds[0], childPostB.ident)
    compare(parentPost.childrenIds[1], childPostA.ident)
  }
  
  function test_findPostIndex() {
    subject.addPost(cloneObject(examplePost))
    subject.addPost(cloneObject(childPostA))
    subject.addPost(cloneObject(childPostB))
    compareObject(subject.get(0), childPostB)
    compareObject(subject.get(1), childPostA)
    compareObject(subject.get(2), examplePost)
    compare(subject.findPostIndex("ident", childPostB.ident), 0)
    compare(subject.findPostIndex("ident", childPostA.ident), 1)
    compare(subject.findPostIndex("ident", examplePost.ident), 2)
    compare(subject.findPostIndex("ident", "__noSuchIdent__"), null)
  }
  
  function test_findPost() {
    subject.addPost(cloneObject(examplePost))
    subject.addPost(cloneObject(childPostA))
    subject.addPost(cloneObject(childPostB))
    compareObject(subject.findPost("ident", examplePost.ident), examplePost)
    compareObject(subject.findPost("ident", childPostA.ident), childPostA)
    compareObject(subject.findPost("ident", childPostB.ident), childPostB)
    compare(subject.findPost("ident", "__noSuchIdent__"), null)
  }
  
  function test_findPosts() {
    subject.addPost(cloneObject(examplePost))
    subject.addPost(cloneObject(childPostA))
    subject.addPost(cloneObject(childPostB))
    compareObjectArray(
      subject.findPosts("parentId", examplePost.ident),
      [childPostB, childPostA])
    compareObjectArray(
      subject.findPosts("parentId", childPostA.ident),
      [])
  }
}


