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
;;    * The code generation script that built this file: zproto_codec_clj
;;    ************************************************************************
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
    (HydraMsg/sendHello socket)
    (HydraMsg/recv socket))
  (hello-ok [this post-id]
    (HydraMsg/sendHello_Ok socket post-id)
    (HydraMsg/recv socket))
  (get-tags [this]
    (HydraMsg/sendGet_Tags socket)
    (HydraMsg/recv socket))
  (get-tags-ok [this tags]
    (HydraMsg/sendGet_Tags_Ok socket tags)
    (HydraMsg/recv socket))
  (get-tag [this tag]
    (HydraMsg/sendGet_Tag socket tag)
    (HydraMsg/recv socket))
  (get-tag-ok [this post-id]
    (HydraMsg/sendGet_Tag_Ok socket post-id)
    (HydraMsg/recv socket))
  (get-post [this post-id]
    (HydraMsg/sendGet_Post socket post-id)
    (HydraMsg/recv socket))
  (get-post-ok [this post-id reply-to previous tags timestamp type content]
    (HydraMsg/sendGet_Post_Ok socket post-id reply-to previous tags timestamp type content)
    (HydraMsg/recv socket))
  (goodbye [this]
    (HydraMsg/sendGoodbye socket)
    (HydraMsg/recv socket))
  (goodbye-ok [this]
    (HydraMsg/sendGoodbye_Ok socket)
    (HydraMsg/recv socket))
  (invalid [this]
    (HydraMsg/sendInvalid socket)
    (HydraMsg/recv socket))
  (failed [this reason]
    (HydraMsg/sendFailed socket reason)
    (HydraMsg/recv socket)))

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
