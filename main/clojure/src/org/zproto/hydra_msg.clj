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
  (hello [this]
    [this routing-id])
  (hello-ok [this post-id]
    [this routing-id post-id])
  (get-tags [this]
    [this routing-id])
  (get-tags-ok [this tags]
    [this routing-id tags])
  (get-tag [this tag]
    [this routing-id tag])
  (get-tag-ok [this tag post-id]
    [this routing-id tag post-id])
  (get-post [this post-id]
    [this routing-id post-id])
  (get-post-ok [this post-id reply-to previous tags timestamp type content]
    [this routing-id post-id reply-to previous tags timestamp type content])
  (goodbye [this]
    [this routing-id])
  (goodbye-ok [this]
    [this routing-id])
  (invalid [this]
    [this routing-id])
  (failed [this reason]
    [this routing-id reason]))

(defrecord HydraMsgSocket [socket]
  PHydraMsg
  (hello [this]
    (HydraMsg/sendHello socket))
  (hello [this routing-id]
    (HydraMsg/sendHello socket routing-id))
  (hello-ok [this post-id]
    (HydraMsg/sendHello_Ok socket post-id))
  (hello-ok [this routing-id post-id]
    (HydraMsg/sendHello_Ok socket routing-id post-id))
  (get-tags [this]
    (HydraMsg/sendGet_Tags socket))
  (get-tags [this routing-id]
    (HydraMsg/sendGet_Tags socket routing-id))
  (get-tags-ok [this tags]
    (HydraMsg/sendGet_Tags_Ok socket tags))
  (get-tags-ok [this routing-id tags]
    (HydraMsg/sendGet_Tags_Ok socket routing-id tags))
  (get-tag [this tag]
    (HydraMsg/sendGet_Tag socket tag))
  (get-tag [this routing-id tag]
    (HydraMsg/sendGet_Tag socket routing-id tag))
  (get-tag-ok [this tag post-id]
    (HydraMsg/sendGet_Tag_Ok socket tag post-id))
  (get-tag-ok [this routing-id tag post-id]
    (HydraMsg/sendGet_Tag_Ok socket routing-id tag post-id))
  (get-post [this post-id]
    (HydraMsg/sendGet_Post socket post-id))
  (get-post [this routing-id post-id]
    (HydraMsg/sendGet_Post socket routing-id post-id))
  (get-post-ok [this post-id reply-to previous tags timestamp type content]
    (HydraMsg/sendGet_Post_Ok socket post-id reply-to previous tags timestamp type content))
  (get-post-ok [this routing-id post-id reply-to previous tags timestamp type content]
    (HydraMsg/sendGet_Post_Ok socket routing-id post-id reply-to previous tags timestamp type content))
  (goodbye [this]
    (HydraMsg/sendGoodbye socket))
  (goodbye [this routing-id]
    (HydraMsg/sendGoodbye socket routing-id))
  (goodbye-ok [this]
    (HydraMsg/sendGoodbye_Ok socket))
  (goodbye-ok [this routing-id]
    (HydraMsg/sendGoodbye_Ok socket routing-id))
  (invalid [this]
    (HydraMsg/sendInvalid socket))
  (invalid [this routing-id]
    (HydraMsg/sendInvalid socket routing-id))
  (failed [this reason]
    (HydraMsg/sendFailed socket reason))
  (failed [this routing-id reason]
    (HydraMsg/sendFailed socket routing-id reason)))

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

(defn recv [{:keys [socket]}]
  (HydraMsg/recv socket))
