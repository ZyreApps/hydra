
import QtQuick 2.2


ListModel {
  id: root
  
  dynamicRoles: true // Allow childrenIdents to be set after insertion
  
  // Add a post object to the list after validation.
  // If the post is problematic, an error is printed and it is ignored.
  // Insertion of the post is sorted by timestamp,
  // with the most recent posts appearing first in the list.
  function addPost(post) {
    if (!_validatePost(post)) return
    var index = _findPlacementIndexFor(post)
    insert(index, post)
    _updateChildrenIdentsOf(post, index)
    _updateChildrenIdentsOfParentOf(post)
  }
  
  // Return true if the post is valid, else false
  function _validatePost(post) {
    return true // TODO: implement
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
  
  // Update the childrenIdents array of the given post at the given index.
  function _updateChildrenIdentsOf(post, index) {
    setProperty(index, "childrenIdents",
      _findPostsMapProp("parentIdent", post.ident, "ident"))
  }
  
  // If the given post is a child of a known post,
  // update the childrenIdents array of that post.
  function _updateChildrenIdentsOfParentOf(post) {
    if (post.parentIdent.length > 0) {
      var parentIndex = findPostIndex('ident', post.parentIdent)
      if (parentIndex !== null)
        setProperty(parentIndex, "childrenIdents",
          _findPostsMapProp("parentIdent", post.parentIdent, "ident"))
    }
  }
}
