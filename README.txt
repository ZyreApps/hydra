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

Hydra uses an "opportunistic explorative" model for exchange. That is, rather than aim to fully synchronize two peers from oldest content to newest, it pulls newest content from a peer, and then works back through older content until it has everything new from the peer. This connection can break at any time.

In this model, bandwidth and time are the limiting factors. Peers do not aim for consistency, though they may achieve this in some cases. Rather, peers try to exchange posts in order of priority, with increasing depth over time. The assumption is that the most recent posts are the most valuable.

## Data Storage

This section describes how the Hydra implementation stores its posts and content data. A Hydra service runs in a working directory and uses a lockfile (hydra.lock) to prevent multiple instances from running in the same directory.

The directory has these files:

* hydra.cfg -- a configuration file, generated the first time if needed
* ledger.txt -- a list of all posts held by the node. A post ID is a UUID, and the ledger holds one UUID per line.
* posts/ -- a subdirectory holding all posts. A post file is named by its UUID and contains the post meta data in ZPL (ZeroMQ RFC 4) format.
* contents/ -- a subdirectory holding all contents. A content file is named by its SHA1 digest.
* peers/ -- a subdirectory holding peer status files. A peer status file is named by the peer's UUID and holds the peer meta data in SPL format.

For each peer we store:

* to be defined.

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

## Ownership and Contributing

The contributors are listed in AUTHORS. This project uses the MPL v2 license, see LICENSE.

The contribution policy is the standard ZeroMQ [C4.1 process](http://rfc.zeromq.org/spec:22). Please read this RFC if you have never contributed to a ZeroMQ project.

Hydra uses the [CLASS (C Language Style for Scalabilty)](http://rfc.zeromq.org/spec:21) guide for code style.

To report an issue, use the [Hydra issue tracker](https://github.com/edgenet/hydra/issues) at github.com.
