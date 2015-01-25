
import QtQuick 2.2


ListModel {
  id: root
  
  // Add a post object to the list after validation.
  // If the post is problematic, an error is printed and it is ignored.
  // Insertion of the post is sorted by timestamp,
  // with the most recent posts appearing first in the list.
  function addPost(post) {
    if (!validatePost(post)) return
    insert(findPlacementIndexFor(post), post)
  }
  
  // Return true if the post is valid, else false
  function validatePost(post) {
    return true // TODO: implement
  }
  
  // Return the index of the most recent post that is older
  // than the given post. This is for sorted insertion.
  function findPlacementIndexFor(post) {
    for (var i=0; i < count; ++i) {
      var other = get(i)
      if (other['timestamp'] < post['timestamp'])
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
}
