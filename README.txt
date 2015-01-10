.set GIT=https://github.com/edgenet/hydra

# Hydra

[![Build Status](https://travis-ci.org/edgenet/hydra.png)](https://travis-ci.org/edgenet/hydra)

## Contents

.toc 2

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

## The Hydra Protocol

This section describes the Hydra Protocol, which governs how two nodes exchange state.

The Hydra protocol is optimized for WiFi connections and prioritizes bandwidth and reliability over latency. So it is chatty, and does not try to push data opportunistically towards peers.

The protocol is asymmetric, with a "client" fetching posts from a "server". In practice, when two nodes discover each other, they will each act as client to each other's server. That is, a Hydra node runs one server, and N clients, at any point in time.

### Node Discovery

Hydra uses Zyre (UDP broadcast beacons) for node discovery, using a Zyre header "X-HYDRA", which provides the server's public TCP endpoint.

### Post Synchronization

We assume that it is usually impossible to fetch all posts from a peer, within any given window of opportunity. Thus, Hydra aims to fetch the most interesting posts from a peer. The handshake between the client and the server works as follows:

* The client says HELLO, and the server replies with HELLO-OK, giving both nodes the chance to identify each other.
 
* The client tells the server what range of posts it already has for the server. If the server is unknown to the client, or has never sent it any posts, this range is empty. Otherwise it consists of two post IDs, an "oldest" and a "newest".

* The server replies with the number of posts it can offer the client, both newer than the range, and older than the range.

* The client can then request posts, always extending the range either towards newer posts, or towards older posts.

* Since posts can exist across many servers, clients first fetch the post ID metadata, and then fetch the content as desired.

* Clients fetch content on a chunk-by-chunk basis. This prevents memory overflow when sending large files.

* The client can also decide to start from scratch and request the newest posts from the server, if the gap is too large.

.pull src/hydra_proto.bnf

## The Hydra API

This section describes the API provided by the current Hydra implementation, and is meant for developers who wish to use Hydra in their applications.

TBD.

## Implementation Notes

A Hydra service runs in a working directory and uses a lockfile (hydra.lock) to prevent multiple instances from running in the same directory.

This Hydra implementation stores posts in a subdirectory called posts, with one text file per post, in ZPL format (ZeroMQ RFC 4). Post files are named "yyyy-mm-dd(nnnnnnnnn)", consisting of the date the post was created on disk (not the post timestamp), and a 9-digit sequence number (the most decimal digits that will fit into a 32-bit integer).

The Hydra working directory has these files:

* hydra.cfg -- a configuration file, generated the first time if needed.
* posts/ -- a subdirectory holding all posts.
* peers/ -- a subdirectory holding peer status files. A peer status file is named by the peer's unique ID and holds the peer meta data in SPL format.

## Ownership and Contributing

The contributors are listed in AUTHORS. This project uses the MPL v2 license, see LICENSE.

The contribution policy is the standard ZeroMQ [C4.1 process](http://rfc.zeromq.org/spec:22). Please read this RFC if you have never contributed to a ZeroMQ project.

Hydra uses the [CLASS (C Language Style for Scalabilty)](http://rfc.zeromq.org/spec:21) guide for code style.

To report an issue, use the [Hydra issue tracker](https://github.com/edgenet/hydra/issues) at github.com.
