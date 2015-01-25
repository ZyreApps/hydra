
import QtQuick 2.2


ListModel {
  id: root
  
  dynamicRoles: true // Allow childrenIds to be set after insertion
  
  // Add a post object to the list after validation.
  // If the post is problematic, an error is printed and it is ignored.
  // Insertion of the post is sorted by timestamp,
  // with the most recent posts appearing first in the list.
  function addPost(post) {
    if (!_validatePost(post)) return
    var index = _findPlacementIndexFor(post)
    insert(index, post)
    _updateChildrenIdsOf(post, index)
    _updateChildrenIdsOfParentOf(post)
  }
  
  // Return true if the post is valid, else false
  function _validatePost(post) {
    if(typeof post.parentId    !== "string") return _invalidPost(post, "parentId")
    if(typeof post.ident       !== "string") return _invalidPost(post, "ident")
    if(typeof post.subject     !== "string") return _invalidPost(post, "subject")
    if(typeof post.content     !== "string") return _invalidPost(post, "content")
    if(typeof post.timestamp   !== "string") return _invalidPost(post, "timestamp")
    return true
  }
  
  // Print an error for invalid post property and return false
  function _invalidPost(post, key) {
    console.error("ERROR: post with invalid key '%1':".arg(key),
      JSON.stringify(post))
    return false
  }
  
  // Return the index of the most recent post that is older
  // than the given post. This is for sorted insertion.
  function _findPlacementIndexFor(post) {
    for (var i=0; i < count; ++i) {
      var other = get(i)
      if (other.timestamp < post.timestamp)
        return i
    }
    return count
  }
  
  // Return the first (most recent) post whose property
  // at the given key equals the given value.
  function findPost(key, value) {
    for (var i=0; i < count; ++i) {
      var post = get(i)
      if (value === post[key])
        return post
    }
    return null
  }
  
  // Return the index of the first (most recent) post whose property
  // at the given key equals the given value.
  function findPostIndex(key, value) {
    for (var i=0; i < count; ++i) {
      var post = get(i)
      if (value === post[key])
        return i
    }
    return null
  }
  
  // Return the array of all posts whose property
  // at the given key equals the given value,
  // sorted by timestamp with the most recent first.
  function findPosts(key, value) {
    var ary = []
    for (var i=0; i < count; ++i) {
      var post = get(i)
      if (value === post[key])
        ary.push(post)
    }
    return ary
  }
  
  // Collect the value of the given map_key for of all posts whose property
  // at the given key equals the given value,
  // sorted by timestamp with the most recent first.
  function _findPostsMapProp(key, value, map_key) {
    var ary = []
    for (var i=0; i < count; ++i) {
      var post = get(i)
      if (value === post[key])
        ary.push(post[map_key])
    }
    return ary
  }
  
  // Update the childrenIds array of the given post at the given index.
  function _updateChildrenIdsOf(post, index) {
    setProperty(index, "childrenIds",
      _findPostsMapProp("parentId", post.ident, "ident"))
  }
  
  // If the given post is a child of a known post,
  // update the childrenIds array of that post.
  function _updateChildrenIdsOfParentOf(post) {
    if (post.parentId.length > 0) {
      var parentIndex = findPostIndex('ident', post.parentId)
      if (parentIndex !== null)
        setProperty(parentIndex, "childrenIds",
          _findPostsMapProp("parentId", post.parentId, "ident"))
    }
  }
}
