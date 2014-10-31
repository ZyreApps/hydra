(ns server
  (:require [zeromq.zmq           :as zmq]
            [org.zproto.hydra-msg :as msg])
  (:import [org.zproto HydraMsg]))

(alter-var-root #'*out* (constantly *out*))

(declare get-latest-post)
(declare get-all-tags)
(declare get-single-tag)


(defn get-latest-post
  [& more]
  ;;
  ;; Fill in the blanks
  ;;
  (println more))
(defn get-all-tags
  [& more]
  ;;
  ;; Fill in the blanks
  ;;
  (println more))
(defn get-single-tag
  [& more]
  ;;
  ;; Fill in the blanks
  ;;
  (println more))


(defprotocol HydraServer
  (hello    [this])
  (get-tags [this])
  (get-tag  [this tag])
  (get-post [this post-id])
  (goodbye  [this])
  (invalid  [this]))

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

(defrecord Server [socket state]
  HydraServer
  (hello [this]
    ;; (next-state state :start :connected)
    (let [response (HydraMsg. HydraMsg/HELLO_OK)]
      (get-latest-post response)
      (.send response socket)
      (.destroy response)
      ))

  (get-tags [this]
    (ensure-state state :connected)
    (let [response (HydraMsg. HydraMsg/GET_TAGS_OK)]
      (get-all-tags response)
      (.send response socket)
      (.destroy response)
      ))

  (get-tag [this tag-id]
    (ensure-state state :connected)
    (let [response (HydraMsg. HydraMsg/GET_TAG_OK)]
      (get-single-tag response tag-id)
      (.send response socket)
      (.destroy response)
      ))

  (invalid [this]
    (.send (HydraMsg. HydraMsg/INVALID) socket))
  )





(defn match-msg
  [server ^HydraMsg msg]
  (let [id (.id msg)]
    (println (= id HydraMsg/HELLO))
    (cond
     (= id HydraMsg/HELLO)    (hello    server)
     (= id HydraMsg/GET_TAGS) (get-tags server)
     (= id HydraMsg/GET_TAG)  (get-tag  server (.tag msg))

     :default                 (invalid  server)

     ;; HydraMsg/GET_POST
     ;; HydraMsg/GET_POST_OK
     ;; HydraMsg/GOODBYE
     ;; HydraMsg/GOODBYE_OK
     ;; HydraMsg/INVALID
     ;; HydraMsg/FAILED
     ))
  )

(defn server-loop
  [socket]
  (let [server (Server. socket (atom :start))]
    (loop []
      (when-let [received (msg/recv socket)]
        (try
          (match-msg server received)
          (catch Exception e
            (.printStackTrace e))
          ))
      (if (not (.isInterrupted (Thread/currentThread)))
        (recur)
        (println "Server shutdown")
        ))))


(comment
  (def c (msg/client-socket "tcp://127.0.0.1:12345"))
  (def s (msg/server-socket "tcp://127.0.0.1:12345"))
  (def sf (future (server-loop s)))
  (server-loop s)
  (msg/hello c)
  )
