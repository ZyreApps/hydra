;;  =========================================================================
;;    HydraProto - The Hydra Protocol
;;
;;    ** WARNING *************************************************************
;;    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
;;    your changes at the next build cycle. This is great for temporary printf
;;    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
;;    for commits are:
;;
;;    * The XML model used for this code generation: hydra_proto.xml
;;    * The code generation script that built this file: zproto_codec_c
;;    ************************************************************************
;;    Copyright (c) the Contributors as noted in the AUTHORS file.       
;;    This file is part of zbroker, the ZeroMQ broker project.           
;;                                                                       
;;    This Source Code Form is subject to the terms of the Mozilla Public
;;    License, v. 2.0. If a copy of the MPL was not distributed with this
;;    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
;;    =========================================================================

(ns org.zproto.hydra-proto
  (:require [zeromq.zmq :as zmq])
  (:import [org.zproto HydraProto]))

(defprotocol PHydraProto
  (hello [this identity nickname]
    [this routing-id identity nickname])
  (hello-ok [this post-id identity nickname]
    [this routing-id post-id identity nickname])
  (get-post [this post-id]
    [this routing-id post-id])
  (get-post-ok [this post-id reply-to previous tags timestamp digest type content]
    [this routing-id post-id reply-to previous tags timestamp digest type content])
  (get-tags [this]
    [this routing-id])
  (get-tags-ok [this tags]
    [this routing-id tags])
  (get-tag [this tag]
    [this routing-id tag])
  (get-tag-ok [this tag post-id]
    [this routing-id tag post-id])
  (goodbye [this]
    [this routing-id])
  (goodbye-ok [this]
    [this routing-id])
  (error [this status reason]
    [this routing-id status reason]))

(defrecord HydraProtoSocket [socket]
  PHydraProto
  (hello [this identity nickname]
    (HydraProto/sendHello socket identity nickname))
  (hello [this routing-id identity nickname]
    (HydraProto/sendHello socket routing-id identity nickname))
  (hello-ok [this post-id identity nickname]
    (HydraProto/sendHello_Ok socket post-id identity nickname))
  (hello-ok [this routing-id post-id identity nickname]
    (HydraProto/sendHello_Ok socket routing-id post-id identity nickname))
  (get-post [this post-id]
    (HydraProto/sendGet_Post socket post-id))
  (get-post [this routing-id post-id]
    (HydraProto/sendGet_Post socket routing-id post-id))
  (get-post-ok [this post-id reply-to previous tags timestamp digest type content]
    (HydraProto/sendGet_Post_Ok socket post-id reply-to previous tags timestamp digest type content))
  (get-post-ok [this routing-id post-id reply-to previous tags timestamp digest type content]
    (HydraProto/sendGet_Post_Ok socket routing-id post-id reply-to previous tags timestamp digest type content))
  (get-tags [this]
    (HydraProto/sendGet_Tags socket))
  (get-tags [this routing-id]
    (HydraProto/sendGet_Tags socket routing-id))
  (get-tags-ok [this tags]
    (HydraProto/sendGet_Tags_Ok socket tags))
  (get-tags-ok [this routing-id tags]
    (HydraProto/sendGet_Tags_Ok socket routing-id tags))
  (get-tag [this tag]
    (HydraProto/sendGet_Tag socket tag))
  (get-tag [this routing-id tag]
    (HydraProto/sendGet_Tag socket routing-id tag))
  (get-tag-ok [this tag post-id]
    (HydraProto/sendGet_Tag_Ok socket tag post-id))
  (get-tag-ok [this routing-id tag post-id]
    (HydraProto/sendGet_Tag_Ok socket routing-id tag post-id))
  (goodbye [this]
    (HydraProto/sendGoodbye socket))
  (goodbye [this routing-id]
    (HydraProto/sendGoodbye socket routing-id))
  (goodbye-ok [this]
    (HydraProto/sendGoodbye_Ok socket))
  (goodbye-ok [this routing-id]
    (HydraProto/sendGoodbye_Ok socket routing-id))
  (error [this status reason]
    (HydraProto/sendError socket status reason))
  (error [this routing-id status reason]
    (HydraProto/sendError socket routing-id status reason)))


;;
;; message setters
;;

(defn id! [^HydraProto msg id]
  (.setId msg id))

(defn identity! [^HydraProto msg format & opts]
 (.setIdentity msg format (object-array opts)))

(defn nickname! [^HydraProto msg format & opts]
 (.setNickname msg format (object-array opts)))

(defn post-id! [^HydraProto msg format & opts]
 (.setPost_Id msg format (object-array opts)))

(defn reply-to! [^HydraProto msg format & opts]
 (.setReply_To msg format (object-array opts)))

(defn previous! [^HydraProto msg format & opts]
 (.setPrevious msg format (object-array opts)))

(defn tags! [^HydraProto msg format & opts]
 (.setTags msg format (object-array opts)))

(defn timestamp! [^HydraProto msg format & opts]
 (.setTimestamp msg format (object-array opts)))

(defn digest! [^HydraProto msg format & opts]
 (.setDigest msg format (object-array opts)))

(defn type! [^HydraProto msg format & opts]
 (.setType msg format (object-array opts)))

(defn content! [^HydraProto msg content]
 (.setContent msg content))

(defn tag! [^HydraProto msg format & opts]
 (.setTag msg format (object-array opts)))

(defn status! [^HydraProto msg status]
 (.setStatus msg status))

(defn reason! [^HydraProto msg format & opts]
 (.setReason msg format (object-array opts)))

(def context (zmq/context))

(defn client-socket [endpoint]
  (let [socket (doto (zmq/socket context :dealer)
                 (zmq/set-receive-timeout 1000)
                 (zmq/connect endpoint))]
    (->HydraProtoSocket socket)))

(defn server-socket [endpoint]
  (let [socket (doto (zmq/socket context :router)
                 (zmq/set-receive-timeout 1000)
                 (zmq/bind endpoint))]
     (->HydraProtoSocket socket)))

(defn recv [{:keys [socket]}]
  (HydraProto/recv socket))
