(ns org.zproto.hydra-server
  (:require [zeromq.zmq           :as zmq]
            [org.zproto.hydra-msg :as msg])
  (:import [org.zproto HydraMsg]))

(alter-var-root #'*out* (constantly *out*))

(defprotocol HydraServer
  (hello    [this routing-id msg])
  (get-tags [this routing-id msg])
  (get-tag  [this routing-id msg])
  (get-post [this routing-id msg])
  (goodbye  [this routing-id msg])
  (invalid  [this routing-id msg]))

(defprotocol HydraServerBackend
  (get-latest-post [this])
  (get-single-post [this post-id])
  (get-all-tags [this])
  (get-single-tag [this tag-id]))

(defn next-state
  [atom routing-id expected-state next-state]
  (swap! atom
         (fn [states]
           (let [current-state (get states routing-id)]
             (if (= current-state expected-state)
               (assoc states routing-id next-state)
               (throw (RuntimeException. (str "Expected to have "
                                              expected-state
                                              " state, but had "
                                              current-state))))))))

(defn maybe-setup-session [state routing-id]
  (if (get state routing-id)
    state
    (assoc state routing-id :start)))

(defrecord Server [socket state backend]
  HydraServer
  (hello [this routing-id msg]
    (let [post-id (get-latest-post backend)]
      (msg/hello-ok socket routing-id post-id)
      (next-state state routing-id :start :connected)))

  (get-tags [this routing-id msg]
    (let [tags (get-all-tags backend)]
      (msg/get-tags-ok socket routing-id tags)))

  (get-tag [this routing-id msg]
    (if-let [post-id (get-single-tag backend (.tag ^HydraMsg msg))]
      (msg/get-tag-ok socket routing-id (.tag ^HydraMsg msg) post-id)
      (msg/failed socket routing-id (format "no post for tag %s" (.tag ^HydraMsg msg)))))

  (get-post [this routing-id msg]
    (if-let [post-data (get-single-post backend (.post_id ^HydraMsg msg))]
      (apply msg/get-post-ok socket routing-id post-data)
      (msg/failed socket
                  routing-id
                  (format "post not found: %s" (.post_id ^HydraMsg msg)))))

  (invalid [this routing-id msg]
    (msg/invalid socket routing-id))

  (goodbye [this routing-id msg]
    (msg/goodbye-ok socket routing-id)))


(def state-events
  {:start {HydraMsg/HELLO hello
           :*       invalid}
   :connected {HydraMsg/GET_TAGS get-tags
               HydraMsg/GET_TAG  get-tag
               HydraMsg/GET_POST get-post
               HydraMsg/GOODBYE  goodbye
               :* invalid}})

(def select-handler
  (memoize
   (fn [state event-id]
     (or (get-in state-events [state event-id])
         (get-in state-events [state :*])))))

(defn match-msg
  [{:keys [state] :as server} ^HydraMsg msg]
  (let [id (.id msg)
        routing-id (.routingId msg)
        initialized-state (swap! state maybe-setup-session routing-id)]
    (let [handler (select-handler (get initialized-state routing-id) id)]
      (handler server routing-id msg))))

(defn server-loop
  [socket backend]
  (let [server (Server. socket (atom {}) backend)]
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
