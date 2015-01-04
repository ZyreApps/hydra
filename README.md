.set GIT=https://github.com/edgenet/hydra

# Hydra

[![Build Status](https://travis-ci.org/edgenet/hydra.png)](https://travis-ci.org/edgenet/hydra)

## Contents

.toc 2

## Overview

Hydra is a simple protocol and stack for opportunistic sharing of events and content.

The goal of the Hydra project is to explore and learn how to share knowledge and information across short-lived wireless networks between mobile users. Hydra exploits "proximity networks", that is the physical closeness of individuals. The Hydra project is part of the edgenet experiment to design and build decentralized networks, with no dependency on broadband Internet nor centralized services.

Plausibly, Hydra could evolve into a fully decentralized social network, closely mapping and amplifying existing human protocols for knowledge and information sharing. However at this stage, Hydra is simply a model for exchanging cat photos.

Hydra does make some core assumptions about technology and platform. It assumes that:

* The platform is a mobile smartphone or tablet, probably running Android;
* Connectivity is wireless, TCP and UDP over WiFi. Perhaps discovery over BLE.
* Peers discover and talk to each other opportunistically, as individuals move around.

Not that Hydra is not a mesh network and does no forwarding or routing. This is deliberate. We assume that peers move around too rapidly for routing knowledge to ever be valid. All a peer can ever know is "I can talk to this other peer, for now."

Hydra is aimed, in its current incarnation, at technical conferences, weddings, parties, and funerals. The participants are in rough proximity, for a period of hours or days. They share a strong set of interests. They implicitly trust each other, yet do not care about identities. They mainly want to share photos and comments on photos.

## The Hydra Data Model

In the Hydra model, an atom of information or content is a "post". A post has these properties:

* A universally unique ID (UUID), which is randomly generated.
* A timestamp that indicates when it was originally created.
* A content MIME type (e.g. "image/jpg").
* Optionally, a parent post, allowing posts to be nested.
* Optionally, a previous post, allowing posts to be chained.
* A content, which is a blob of text, image, or other data.

Hydra currently makes no attempt to authenticate posts, identify participants, nor rank or rate posts in any way. These are known weaknesses. We expect to entirely redesign the Hydra data model at a later date.

Hydra uses an "opportunistic explorative" model for exchange. That is, rather than aim to fully synchronize two peers, it allows each peer to explore the posts held by the other peer. 

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


## Ownership and License

The contributors are listed in AUTHORS. This project uses the MPL v2 license, see LICENSE.

CZMQ uses the [C4.1 (Collective Code Construction Contract)](http://rfc.zeromq.org/spec:22) process for contributions.

CZMQ uses the [CLASS (C Language Style for Scalabilty)](http://rfc.zeromq.org/spec:21) guide for code style.

To report an issue, use the [CZMQ issue tracker](https://github.com/zeromq/czmq/issues) at github.com.

## Using Hydra





