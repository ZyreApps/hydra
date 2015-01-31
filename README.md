
<A name="toc1-2" title="Hydra" />
# Hydra

[![Build Status](https://travis-ci.org/edgenet/hydra.png)](https://travis-ci.org/edgenet/hydra)

<A name="toc2-7" title="Contents" />
## Contents


**<a href="#toc2-12">Building Hydra</a>**

**<a href="#toc2-18">Building Hydra QML app</a>**

**<a href="#toc2-28">Testing</a>**

**<a href="#toc2-41">Overview</a>**

**<a href="#toc2-60">The Hydra Data Model</a>**

**<a href="#toc2-97">The Hydra Protocol</a>**
&emsp;<a href="#toc3-106">Node Discovery</a>
&emsp;<a href="#toc3-111">Post Synchronization</a>

**<a href="#toc2-230">The Hydra API</a>**

**<a href="#toc2-237">Implementation Notes</a>**

**<a href="#toc2-250">Ownership and Contributing</a>**

<A name="toc2-12" title="Building Hydra" />
## Building Hydra

To build the Hydra C stack do the following:

    git clone git://github.com/zeromq/libzmq.git
    git clone git://github.com/zeromq/czmq.git
    git clone git://github.com/zeromq/zyre.git
    git clone git://github.com/edgenet/hydra.git
    for project in libzmq czmq zyre hydra; do
        cd $project
        ./autogen.sh && ./configure && make check
        sudo make install && ldconfig
        cd ..
    done

<A name="toc2-18" title="Building Hydra QML app" />
## Building Hydra QML app

For documentation on building the QML app, see the README.md under the app/ subdirectory.

<A name="toc2-28" title="Testing" />
## Testing

To test from the command line, run the hydrad executable like this:

    hydrad -t       #  Create some test posts
    hydrad          #  Run a node in default .hydra directory
    hydrad node1    #  Run a node in ./node1 directory (creates it if needed)
    hydrad -i ...   #  Use IPC, if you don't have WiFi/LAN enabled
    hydrad -v ...   #  Enable verbose animation / tracing

Note -i option needs at least one node to run using default .hydra directory. 

<A name="toc2-41" title="Overview" />
## Overview

Hydra is a simple protocol and stack for opportunistic sharing of events and content.

The goal of the Hydra project is to explore and learn how to share knowledge and information across short-lived wireless networks between mobile users. Hydra exploits "proximity networks", that is the physical closeness of individuals. The Hydra project is part of the edgenet experiment to design and build decentralized networks, with no dependency on broadband Internet nor centralized services.

Plausibly, Hydra could evolve into a fully decentralized social network, closely mapping and amplifying existing human protocols for knowledge and information sharing. However at this stage, Hydra is simply a model for exchanging cat photos.

Hydra does make some core assumptions about technology and platform. It assumes that:

* The platform is a mobile smartphone or tablet, probably running Android;
* Connectivity is wireless, TCP and UDP over WiFi.
* Peers discover and talk to each other opportunistically, as individuals move around.

Not that Hydra is not a mesh network and does no forwarding or routing. This is deliberate. We assume that peers move around too rapidly for routing knowledge to ever be valid. All a peer can ever know is "I can talk to this other peer, for now."

Hydra is aimed, in its current incarnation, at technical conferences, weddings, parties, and funerals. The participants are in rough proximity, for a period of hours or days. They share a strong set of interests. They implicitly trust each other, yet do not care about identities. They mainly want to share photos and comments on photos.

<A name="toc2-60" title="The Hydra Data Model" />
## The Hydra Data Model

This section describes the Hydra Data model, which underpins the rest of the design.

A Hydra network consists of a set of self-identified *nodes*, which produce and share *posts*. A post is a piece of content (typically a photo or some text) with metadata.

Every node has a self-generated UUID. The node configuration is held in hydra.cfg, which has this format:

    hydra
        identity = "FB04239C786E480BB27007576627C502"
        nickname = "Anonymous"

//TODO: instead of a UUID, generate a CURVE certificate and use the public key as node ID. Then, we can sign posts with our certificate to ensure authenticity.//

Posts have a permanent unique identifier which is the SHA1 digest of the post metadata, calculated thus:

    post_id = sha1 (subject ":" timestamp ":" parent_id ":" mime_type ":" digest)

A post has these immutable properties, apart from its identifier:

* A subject line (a long string).
* The creation timestamp, in ISO 8601 UTC format: yyyy-mm-ddThh:mm:ssZ
* An optional parent post ID, allowing posts to be nested by a presentation layer.
* The MIME type (e.g. "image/jpg") of the content.
* The SHA1 digest of the content.
* The content size, zero or greater.
* The content itself, zero or more octets.

When a post is held by multiple nodes, it shall have the same ID on all nodes.

Every post has a content of zero or more octets. Hydra does not support multipart contents.

//TODO: extend to support multiple content parts.//

A Hydra node can store posts and content in any format. That is, the protocol makes no assumptions about the storage model.

<A name="toc2-97" title="The Hydra Protocol" />
## The Hydra Protocol

This section describes the Hydra Protocol, which governs how two nodes exchange state.

The Hydra protocol is optimized for WiFi connections and prioritizes bandwidth and reliability over latency. So it is chatty, and does not try to push data opportunistically towards peers.

The protocol is asymmetric, with a "client" fetching posts from a "server". In practice, when two nodes discover each other, they will each act as client to each other's server. That is, a Hydra node runs one server, and N clients, at any point in time.

<A name="toc3-106" title="Node Discovery" />
### Node Discovery

Hydra uses Zyre (UDP broadcast beacons) for node discovery, using a Zyre header "X-HYDRA", which provides the server's public TCP endpoint.

<A name="toc3-111" title="Post Synchronization" />
### Post Synchronization

We assume that it is usually impossible to fetch all posts from a peer, within any given window of opportunity. Thus, Hydra aims to fetch the most interesting posts from a peer. The handshake between the client and the server works as follows:

* The client says HELLO, and the server replies with HELLO-OK, giving both nodes the chance to identify each other.
 
* The client tells the server what range of posts it already has for the server. If the server is unknown to the client, or has never sent it any posts, this range is empty. Otherwise it consists of two post IDs, an "oldest" and a "newest".

* The server replies with the number of posts it can offer the client, both newer than the range, and older than the range.

* The client can then request posts, always extending the range either towards newer posts, or towards older posts.

* Since posts can exist across many servers, clients first fetch the post ID metadata, and then fetch the content as desired.

* Clients fetch content on a chunk-by-chunk basis. This prevents memory overflow when sending large files.

* The client can also decide to start from scratch and request the newest posts from the server, if the gap is too large.

The following ABNF grammar defines the The Hydra Protocol:

    hydra = hello *( get-post ) [ goodbye ]
    hello = c:hello ( s:hello-ok / s:invalid / s:failed )
    get-post = c:get-post ( s:get-post-ok / s:invalid / s:failed )
    goodbye = c:goodbye ( s:goodbye-ok / s:invalid / s:failed )

    ;  Open new connection, provide client credentials.                      

    HELLO           = signature %d1 identity nickname
    signature       = %xAA %xA0             ; two octets
    identity        = string                ; Client identity
    nickname        = string                ; Client nickname

    ;  Accept new connection, provide server credentials.                    

    HELLO-OK        = signature %d2 identity nickname
    identity        = string                ; Server identity
    nickname        = string                ; Server nickname

    ;  Client requests next post that is older than the specified post ID. If
    ;  the post ID is "HEAD", fetches the newest post that the server has.   

    NEXT-OLDER      = signature %d3 ident
    ident           = string                ; Client's oldest post ID

    ;  Client requests next post that is newer than the specified post ID. If
    ;  the post ID is "TAIL", fetches the oldest post that the server has.   

    NEXT-NEWER      = signature %d4 ident
    ident           = string                ; Client's newest post ID

    ;  Server returns a post identity to the client. This command does not   
    ;  provide all metadata, only the post identity string. Clients can then 
    ;  filter out posts they already have.                                   

    NEXT-OK         = signature %d5 ident
    ident           = string                ; Post identifier

    ;  Server signals that it has no (more) posts for the client.            

    NEXT-EMPTY      = signature %d6

    ;  Client requests the metadata for the current post. A META command only
    ;  makes sense after a NEXT-OLDER or NEXT-NEWER with a successful NEXT-OK
    ;  from the server.                                                      

    META            = signature %d7

    ;  Server returns the metadata for the current post (as returned by      
    ;  NEXT-OK).                                                             

    META-OK         = signature %d8 subject timestamp parent_id digest mime_type content_size
    subject         = longstr               ; Subject line
    timestamp       = string                ; Post creation timestamp
    parent id       = string                ; Parent post ID, if any
    digest          = string                ; Content SHA1 digest
    mime type       = string                ; Content MIME type
    content size    = number-8              ; Content size, octets

    ;  Client fetches a chunk of content data from the server, for the       
    ;  current post (as returned by NEXT-OK).                                

    CHUNK           = signature %d9 offset octets
    offset          = number-8              ; Chunk offset in file
    octets          = number-4              ; Maximum chunk size to fetch

    ;  Return a chunk of post content.                                       

    CHUNK-OK        = signature %d10 offset content
    offset          = number-8              ; Chunk offset in file
    content         = chunk                 ; Content data chunk

    ;  Close the connection politely                                         

    GOODBYE         = signature %d11

    ;  Handshake a connection close                                          

    GOODBYE-OK      = signature %d12

    ;  Command failed for some specific reason                               

    ERROR           = signature %d13 status reason
    status          = number-2              ; 3-digit status code
    reason          = string                ; Printable explanation

    ; A chunk has 4-octet length + binary contents
    chunk           = number-4 *OCTET

    ; Strings are always length + text contents
    string          = number-1 *VCHAR
    longstr         = number-4 *VCHAR

    ; Numbers are unsigned integers in network byte order
    number-1        = 1OCTET
    number-2        = 2OCTET
    number-4        = 4OCTET
    number-8        = 8OCTET

<A name="toc2-230" title="The Hydra API" />
## The Hydra API

This section describes the API provided by the current Hydra implementation, and is meant for developers who wish to use Hydra in their applications.

TBD.

<A name="toc2-237" title="Implementation Notes" />
## Implementation Notes

A Hydra service runs in a working directory and uses a lockfile (hydra.lock) to prevent multiple instances from running in the same directory.

This Hydra implementation stores posts in a subdirectory called posts, with one text file per post, in ZPL format (ZeroMQ RFC 4). Post files are named "yyyy-mm-dd(nnnnnnnnn)", consisting of the date the post was created on disk (not the post timestamp), and a 9-digit sequence number (the most decimal digits that will fit into a 32-bit integer).

The Hydra working directory has these files:

* hydra.cfg -- a configuration file, generated the first time if needed.
* posts/ -- a subdirectory holding all posts.
* peers/ -- a subdirectory holding peer status files. A peer status file is named by the peer's unique ID and holds the peer meta data in SPL format.

<A name="toc2-250" title="Ownership and Contributing" />
## Ownership and Contributing

The contributors are listed in AUTHORS. This project uses the MPL v2 license, see LICENSE.

The contribution policy is the standard ZeroMQ [C4.1 process](http://rfc.zeromq.org/spec:22). Please read this RFC if you have never contributed to a ZeroMQ project.

Hydra uses the [CLASS (C Language Style for Scalabilty)](http://rfc.zeromq.org/spec:21) guide for code style.

To report an issue, use the [Hydra issue tracker](https://github.com/edgenet/hydra/issues) at github.com.
