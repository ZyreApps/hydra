;;  =========================================================================
;;    hydra_server implementation
;;
;;  =========================================================================
(ns org.zproto.hydra-server-impl
  (:require [org.zproto.hydra-server :as server]
            [org.zproto.hydra-msg :as m]))

(def backend
  (reify server/HydraServerBackend
    (get-latest-post [this msg])
    (get-all-tags [this msg])
    (get-single-tag [this msg tag])
    (get-single-post [this msg post-id])))

(defn -main [& [endpoint]]
  (println (format "starting up hydra_server at %s" endpoint))
  (server/server-loop (m/server-socket endpoint) backend))
