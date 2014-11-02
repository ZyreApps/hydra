(ns org.zproto.hydra-server
  (:require [zeromq.zmq           :as zmq]
            [org.zproto.hydra-msg :as msg])
  (:import [org.zproto HydraMsg]))

(alter-var-root #'*out* (constantly *out*))

(defprotocol HydraServer
  (hello    [this routing-id])
  (get-tags [this])
  (get-tag  [this tag])
  (get-post [this post-id])
  (goodbye  [this])
  (invalid  [this]))

(defprotocol HydraServerBackend
  (get-latest-post [this])
  (get-single-post [this post-id])
  (get-all-tags [this])
  (get-single-tag [this tag-id]))

(defn next-state
  [atom expected-state next-state]
  (swap! atom
         (fn [st]
           (if (= st expected-state)
             next-state
             (throw (RuntimeException. (str "Expected to have "
                                            expected-state
                                            " state, but had "
                                            next-state)))))))

(defn ensure-state
  [atom expected-state]
  (when (not (= @atom expected-state))
    (throw (RuntimeException.
            (str "Expected to have "
                 expected-state
                 " state, but had "
                 next-state)))))

(defrecord Server [socket state backend]
  HydraServer
  (hello [this routing-id]
    ;; (next-state state :start :connected)
    (let [post-id (get-latest-post backend)]
      (msg/hello-ok socket routing-id post-id)))

  (get-tags [this]
    (ensure-state state :connected)
    (let [response (HydraMsg. HydraMsg/GET_TAGS_OK)]
      (get-all-tags response)
      (.send response socket)
      (.destroy response)))

  (get-tag [this tag-id]
    (ensure-state state :connected)
    (let [response (HydraMsg. HydraMsg/GET_TAG_OK)]
      (get-single-tag response tag-id)
      (.send response socket)
      (.destroy response)))

  (invalid [this]
    (.send (HydraMsg. HydraMsg/INVALID) socket)))





(defn match-msg
  [server ^HydraMsg msg]
  (let [id (.id msg)
        routing-id (.routingId msg)]
    (cond
     (= id HydraMsg/HELLO)    (hello    server routing-id)
     (= id HydraMsg/GET_TAGS) (get-tags server)
     (= id HydraMsg/GET_TAG)  (get-tag  server (.tag msg))

     :default                 (invalid  server))))

(defn server-loop
  [socket backend]
  (let [server (Server. socket (atom :start) backend)]
    (loop []
      (when-let [received (msg/recv socket)]
        (try
          (match-msg server received)
          (catch Exception e
            (.printStackTrace e))))
      (if (not (.isInterrupted (Thread/currentThread)))
        (recur)
        (println "Server shutdown")))))


(comment
  (def c (msg/client-socket "tcp://127.0.0.1:12345"))
  (def s (msg/server-socket "tcp://127.0.0.1:12345"))
  (def sf (future (server-loop s)))
  (server-loop s)
  (msg/hello c)
  )
