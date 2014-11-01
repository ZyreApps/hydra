;;  =========================================================================
;;    HydraMsg - The Hydra Protocol
;;
;;    ** WARNING *************************************************************
;;    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
;;    your changes at the next build cycle. This is great for temporary printf
;;    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
;;    for commits are:
;;
;;    * The XML model used for this code generation: hydra_msg.xml
;;    * The code generation script that built this file: zproto_codec_c
;;    ************************************************************************
;;    Copyright (c) the Contributors as noted in the AUTHORS file.       
;;    This file is part of zbroker, the ZeroMQ broker project.           
;;                                                                       
;;    This Source Code Form is subject to the terms of the Mozilla Public
;;    License, v. 2.0. If a copy of the MPL was not distributed with this
;;    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
;;    =========================================================================

(ns org.zproto.hydra-msg
  (:require [zeromq.zmq :as zmq])
  (:import [org.zproto HydraMsg]))

(defprotocol PHydraMsg
  (hello [this])
  (hello-ok [this post-id])
  (get-tags [this])
  (get-tags-ok [this tags])
  (get-tag [this tag])
  (get-tag-ok [this post-id])
  (get-post [this post-id])
  (get-post-ok [this post-id reply-to previous tags timestamp type content])
  (goodbye [this])
  (goodbye-ok [this])
  (invalid [this])
  (failed [this reason]))

(defrecord HydraMsgSocket [socket]
  PHydraMsg
  (hello [this]
    (HydraMsg/sendHello socket))
  (hello-ok [this post-id]
    (HydraMsg/sendHello_Ok socket post-id))
  (get-tags [this]
    (HydraMsg/sendGet_Tags socket))
  (get-tags-ok [this tags]
    (HydraMsg/sendGet_Tags_Ok socket tags))
  (get-tag [this tag]
    (HydraMsg/sendGet_Tag socket tag))
  (get-tag-ok [this post-id]
    (HydraMsg/sendGet_Tag_Ok socket post-id))
  (get-post [this post-id]
    (HydraMsg/sendGet_Post socket post-id))
  (get-post-ok [this post-id reply-to previous tags timestamp type content]
    (HydraMsg/sendGet_Post_Ok socket post-id reply-to previous tags timestamp type content))
  (goodbye [this]
    (HydraMsg/sendGoodbye socket))
  (goodbye-ok [this]
    (HydraMsg/sendGoodbye_Ok socket))
  (invalid [this]
    (HydraMsg/sendInvalid socket))
  (failed [this reason]
    (HydraMsg/sendFailed socket reason)))

(defn client-socket [endpoint]
  (let [context (zmq/context)
        socket (doto (zmq/socket context :dealer)
                 (zmq/set-receive-timeout 1000)
                 (zmq/connect endpoint))]
    (->HydraMsgSocket socket)))

(defn server-socket [endpoint]
  (let [context (zmq/context)
        socket (doto (zmq/socket context :router)
                 (zmq/set-receive-timeout 1000)
                 (zmq/bind endpoint))]
    (->HydraMsgSocket socket)))

(defn recv [socket]
  (HydraMsg/recv socket))
