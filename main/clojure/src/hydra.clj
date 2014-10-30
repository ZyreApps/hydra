(ns hydra
  (:require [zeromq.zmq :as zmq])
  (:import [org.zproto HydraMsg]))


(defn simple-hello
  "send a hello message to endpoint and receive the reply."
  [endpoint]
  (let [context (zmq/context)
        output (doto (zmq/socket context :dealer)
                 (zmq/connect endpoint)
                 (zmq/set-receive-timeout 1000))]
    (HydraMsg/sendHello output)
    (HydraMsg/recv output)))
