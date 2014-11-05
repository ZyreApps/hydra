# The Hydra Protocol

[![Build Status](https://travis-ci.org/edgenet/hydra.png)](https://travis-ci.org/edgenet/hydra)

Hydra is a simple protocol for opportunistic sharing of events and content.

The goal of the Hydra project is to explore and learn how to share knowledge and information across short-lived wireless networks between mobile users. Hydra exploits "proximity networks", that is the physical closeness of individuals. The Hydra project is part of the edgenet campaign to design and build decentralized networks, with no dependency on broadband Internet nor centralized services.

Plausibly, Hydra could evolve into a fully decentralized social network, closely mapping and amplifying existing human protocols for knowledge and information sharing. However at this stage, Hydra is simply a model for exchanging chat and perhaps cat photos.

Hydra does make some core assumptions about technology and platform. It assumes that:

* The platform is a mobile smartphone or tablet, probably running Android;
* Connectivity is wireless, TCP and UDP over WiFi. Perhaps discovery over BLE.
* Peers discover and talk to each other opportunistically, as individuals move around.

Not that Hydra is not a mesh network, nor yet an end-user application. It is a model for representing data, a protocol for exchanging it, and in this project, a simple implementation written in C and Java and Clojure that tests the concept over ZeroMQ.

## Use Cases

Hydra is aimed, in its current incarnation, at a technical conference, funeral, wedding, or street party. The participants are in proximity, for a period of hours or days. They share a strong set of interests. They implicitly trust each other, yet do not care about identities. They wish to share primarily photographs, comments on photographs, and perhaps also short video recordings.

## The Hydra Data Model

In the Hydra model, an atom of information or content is a "post". A post has these properties:

* an ID that is a hash of all its other properties
* a timestamp
* a type (e.g. image/jpeg)
* a body (e.g. an attached image)
* a set of zero or more tags
* a parent post id, allowing posts to be nested
* a previous post id, allowing posts to be threaded

Posts are immutable and anonymous. Hydra makes no attempt, today, to:

* define the authenticity of posts
* identify the participants in the network
* rank or rate posts in any way

Hydra uses an "opportunistic explorative" model for exchange. That is, rather than aim to synchronize two peers, it allows each peer to explore the posts held by the other peer. This exploration works in two ways:

* Starting from nothing, one peer can ask another to list all tags it has.
* Starting from a tag, one peer can ask another for the most recent post with that tag.
* Starting from a post, one peer can ask another for the parent post, or previous post.

In this model, bandwidth and time are the limiting factors. Peers do not aim for consistency, though they may achieve this in exceptional cases. Rather, peers try to exchange posts in order of priority, with increasing depth over time. The underlying assumption is that the most recent posts are the most valuable.

## Building Hydra

To build the Hydra C stack, you need first to build and install these projects:

    git clone git://github.com/zeromq/libzmq.git
    git clone git://github.com/zeromq/czmq.git
    for project in libzmq czmq; do
        cd $project
        ./autogen.sh
        ./configure && make check
        sudo make install
        sudo ldconfig
        cd ..
    done

Then use "./autogen.sh && ./configure && make" on Hydra.
    
## Ownership and Contributing

The contributors are listed in AUTHORS. This project uses the MPL v2 license, see LICENSE.

The contribution policy is the standard ZeroMQ [C4.1 process](http://rfc.zeromq.org/spec:22). Please read this RFC if you have never contributed to a ZeroMQ project.

## Areas for Research

* How to deal with maliciously low quality content (spam, fakes, altered data, etc.)
* How to progressively transfer larger files (photos and video)
