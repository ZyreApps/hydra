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
  (hello-ok [this identity nickname]
    [this routing-id identity nickname])
  (status [this oldest newest]
    [this routing-id oldest newest])
  (status-ok [this older newer]
    [this routing-id older newer])
  (header [this which]
    [this routing-id which])
  (header-ok [this identifier subject timestamp parent-post content-digest content-type content-size]
    [this routing-id identifier subject timestamp parent-post content-digest content-type content-size])
  (fetch [this offset octets]
    [this routing-id offset octets])
  (fetch-ok [this offset octets content]
    [this routing-id offset octets content])
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
  (hello-ok [this identity nickname]
    (HydraProto/sendHello_Ok socket identity nickname))
  (hello-ok [this routing-id identity nickname]
    (HydraProto/sendHello_Ok socket routing-id identity nickname))
  (status [this oldest newest]
    (HydraProto/sendStatus socket oldest newest))
  (status [this routing-id oldest newest]
    (HydraProto/sendStatus socket routing-id oldest newest))
  (status-ok [this older newer]
    (HydraProto/sendStatus_Ok socket older newer))
  (status-ok [this routing-id older newer]
    (HydraProto/sendStatus_Ok socket routing-id older newer))
  (header [this which]
    (HydraProto/sendHeader socket which))
  (header [this routing-id which]
    (HydraProto/sendHeader socket routing-id which))
  (header-ok [this identifier subject timestamp parent-post content-digest content-type content-size]
    (HydraProto/sendHeader_Ok socket identifier subject timestamp parent-post content-digest content-type content-size))
  (header-ok [this routing-id identifier subject timestamp parent-post content-digest content-type content-size]
    (HydraProto/sendHeader_Ok socket routing-id identifier subject timestamp parent-post content-digest content-type content-size))
  (fetch [this offset octets]
    (HydraProto/sendFetch socket offset octets))
  (fetch [this routing-id offset octets]
    (HydraProto/sendFetch socket routing-id offset octets))
  (fetch-ok [this offset octets content]
    (HydraProto/sendFetch_Ok socket offset octets content))
  (fetch-ok [this routing-id offset octets content]
    (HydraProto/sendFetch_Ok socket routing-id offset octets content))
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

(defn oldest! [^HydraProto msg format & opts]
 (.setOldest msg format (object-array opts)))

(defn newest! [^HydraProto msg format & opts]
 (.setNewest msg format (object-array opts)))

(defn older! [^HydraProto msg older]
 (.setOlder msg older))

(defn newer! [^HydraProto msg newer]
 (.setNewer msg newer))

(defn which! [^HydraProto msg which]
 (.setWhich msg which))

(defn identifier! [^HydraProto msg format & opts]
 (.setIdentifier msg format (object-array opts)))

(defn subject! [^HydraProto msg format & opts]
 (.setSubject msg format (object-array opts)))

(defn timestamp! [^HydraProto msg format & opts]
 (.setTimestamp msg format (object-array opts)))

(defn parent-post! [^HydraProto msg format & opts]
 (.setParent_Post msg format (object-array opts)))

(defn content-digest! [^HydraProto msg format & opts]
 (.setContent_Digest msg format (object-array opts)))

(defn content-type! [^HydraProto msg format & opts]
 (.setContent_Type msg format (object-array opts)))

(defn content-size! [^HydraProto msg content-size]
 (.setContent_Size msg content-size))

(defn offset! [^HydraProto msg offset]
 (.setOffset msg offset))

(defn octets! [^HydraProto msg octets]
 (.setOctets msg octets))

(defn content! [^HydraProto msg content]
 (.setContent msg content))

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
