(ns org.zproto.hydra-server-test
  (:require [clojure.test :refer :all]
            [org.zproto.hydra-server :as server]
            [org.zproto.hydra-msg :as msg])
  (:import [org.zproto HydraMsg]))

(def test-endpoint "ipc://hydra-server-test")

(def test-server-backend
  (reify server/HydraServerBackend
    (get-latest-post [this]
      "dummy-post-id")
    (get-single-post [this post-id]
      "dummy-post-id")
    (get-all-tags [this]
      ["tag 1" "tag 2" "tag 3"])
    (get-single-tag [this tag-id]
      "tag 1")))

(deftest test-hello-ok
  (let [srv-sock (msg/server-socket test-endpoint)
        cl-sock (msg/client-socket test-endpoint)
        srv (server/->Server srv-sock (atom :start) test-server-backend)]
    (try
      (msg/hello cl-sock)
      (server/match-msg srv (msg/recv srv-sock))
      (let [response (msg/recv cl-sock)]
        (is (= HydraMsg/HELLO_OK
               (.id response)))
        (is (= "dummy-post-id"
               (.post_id response))))
      (finally
        (.close (:socket srv-sock))
        (.close (:socket cl-sock))))))
