(ns hydra
  (:require [zeromq.zmq :as zmq])
  (:import [org.zproto HydraMsg]
           [org.zeromq ZMsg]
           [java.io StringWriter PrintWriter PrintStream OutputStreamWriter]))


(defprotocol HydraClient
  (hello    [this])
  (get-tags [this])
  (get-tag [this tag])
  (get-post [this post-id])
  (goodbye [this]))

(defrecord Client [socket]
  HydraClient
  (hello [this]
    (HydraMsg/sendHello socket)
    (HydraMsg/recv socket))
  (get-tags [this]
    (HydraMsg/sendGet_Tags socket)
    (HydraMsg/recv socket))
  (get-tag [this tag]
    (HydraMsg/sendGet_Tag socket tag)
    (HydraMsg/recv socket))
  (get-post [this post-id]
    (HydraMsg/sendGet_Post socket post-id)
    (HydraMsg/recv socket))
  (goodbye [this]
    (HydraMsg/sendGoodbye socket)
    (HydraMsg/recv socket)))

(defn mk-client [endpoint]
  (let [context (zmq/context)
        socket (doto (zmq/socket context :dealer)
                 (zmq/set-receive-timeout 1000)
                 (zmq/connect endpoint))]
    (->Client socket)))
