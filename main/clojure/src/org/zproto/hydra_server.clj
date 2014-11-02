(ns org.zproto.hydra-server
  (:require [zeromq.zmq           :as zmq]
            [org.zproto.hydra-msg :as msg])
  (:import [org.zproto HydraMsg]))

(alter-var-root #'*out* (constantly *out*))

(defprotocol HydraServer
  (hello    [this routing-id])
  (get-tags [this routing-id])
  (get-tag  [this routing-id tag])
  (get-post [this routing-id post-id])
  (goodbye  [this routing-id])
  (invalid  [this routing-id]))

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

  (get-tags [this routing-id]
    (ensure-state state :connected)
    (let [tags (get-all-tags backend)]
      (msg/get-tags-ok socket routing-id tags)))

  (get-tag [this routing-id tag]
    (ensure-state state :connected)
    (if-let [post-id (get-single-tag backend tag)]
      (msg/get-tag-ok socket routing-id tag post-id)
      (msg/failed socket routing-id (format "no post for tag %s" tag))))

  (get-post [this routing-id post-id]
    (ensure-state state :connected)
    (if-let [post-data (get-single-post backend post-id)]
      (apply msg/get-post-ok socket routing-id post-data)
      (msg/failed socket routing-id (format "post not found: %s" post-id))))

  (invalid [this routing-id]
    (msg/invalid socket routing-id))

  (goodbye [this routing-id]
    (ensure-state state :connected)
    (msg/goodbye-ok socket routing-id)))


(defn match-msg
  [server ^HydraMsg msg]
  (let [id (.id msg)
        routing-id (.routingId msg)]
    (cond
     (= id HydraMsg/HELLO)    (hello    server routing-id)
     (= id HydraMsg/GET_TAGS) (get-tags server routing-id)
     (= id HydraMsg/GET_TAG)  (get-tag  server routing-id (.tag msg))
     (= id HydraMsg/GET_POST) (get-post server routing-id (.post_id msg))
     (= id HydraMsg/GOODBYE)  (goodbye  server routing-id)

     :default                 (invalid  server routing-id))))

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
