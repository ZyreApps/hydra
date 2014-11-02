(ns org.zproto.hydra-server-test
  (:require [clojure.test :refer :all]
            [org.zproto.hydra-server :as server]
            [org.zproto.hydra-msg :as msg])
  (:import [org.zproto HydraMsg]))

(def test-endpoint "ipc://hydra-server-test")

(deftest test-hello-ok
  (let [srv-sock (msg/server-socket test-endpoint)
        cl (msg/client-socket test-endpoint)
        srv (server/->Server srv-sock (atom :start))]
    (try
      (msg/hello cl)
      (server/match-msg srv (msg/recv srv-sock))
      (let [response (msg/recv cl)]
        (is (= HydraMsg/HELLO_OK
               (.id response)))
        (is (= "dummy-id"
               (.post_id response))))
      (finally
        (.close (:socket srv-sock))
        (.close (:socket cl))))))
