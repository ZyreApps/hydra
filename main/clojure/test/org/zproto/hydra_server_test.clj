(ns org.zproto.hydra-server-test
  (:require [clojure.test :refer :all]
            [org.zproto.hydra-server :as server]
            [org.zproto.hydra-msg :as msg])
  (:import [org.zproto HydraMsg]))

(def test-endpoint "inproc://hydra-server-test")

(def dummy-backend
  (reify server/HydraServerBackend
    (get-latest-post [this]
      "dummy-post-id")
    (get-single-post [this post-id]
      (when (= "dummy-post-id" post-id)
        ["dummy-post-id" "none" "none" "tag_2" 123123123 "dummy" "dummy-post"]))
    (get-all-tags [this]
      "tag_1 tag_2 tag_3 tag_4")
    (get-single-tag [this tag-id]
      (when (= tag-id "tag_1")
        [tag-id "dummy-post-id-2"]))))

(defn setup [& [connect?]]
  (let [srv-sock (msg/server-socket test-endpoint)
        cl-sock (msg/client-socket test-endpoint)
        srv (server/->Server srv-sock (atom {}) dummy-backend)]
    (when connect?
      (msg/hello cl-sock)
      (server/match-msg srv (msg/recv srv-sock))
      (msg/recv cl-sock))
    [cl-sock srv-sock srv]))

(defn teardown [cl-sock srv-sock]
  (.close (:socket srv-sock))
  (.close (:socket cl-sock)))

(defn server-client-comm [cl-sock srv srv-sock cl-msg & args]
  (apply cl-msg cl-sock args)
  (server/match-msg srv (msg/recv srv-sock))
  (msg/recv cl-sock))

(deftest test-hello-ok
  (let [[cl-sock srv-sock srv] (setup)]
    (try
      (let [response (server-client-comm cl-sock srv srv-sock msg/hello)]
        (is (= HydraMsg/HELLO_OK
               (.id response)))
        (is (= (server/get-latest-post dummy-backend)
               (.post_id response)))
        (is (= :connected
               (-> @(:state srv) vals first))))
      (finally
        (teardown cl-sock srv-sock)))))

(deftest test-get-tags-ok
  (let [[cl-sock srv-sock srv] (setup :connected)]
    (try
      (let [response (server-client-comm cl-sock srv srv-sock msg/get-tags)]
        (is (= HydraMsg/GET_TAGS_OK
               (.id response)))
        (is (= (server/get-all-tags dummy-backend)
               (.tags response)))
        (is (= :connected
               (-> @(:state srv) vals first))))
      (finally
        (teardown cl-sock srv-sock)))))

(deftest test-get-tag-ok
  (let [[cl-sock srv-sock srv] (setup :connected)]
    (try
      (let [response (server-client-comm cl-sock srv srv-sock msg/get-tag "tag_1")]
        (is (= HydraMsg/GET_TAG_OK
               (.id response)))
        (is (= (server/get-single-tag dummy-backend "tag_1")
               [(.tag response) (.post_id response)]))
        (is (= :connected
               (-> @(:state srv) vals first))))
      (finally
        (teardown cl-sock srv-sock)))))

(deftest test-get-post-ok
  (let [[cl-sock srv-sock srv] (setup :connected)]
    (try
      (let [response (server-client-comm cl-sock srv srv-sock msg/get-post "dummy-post-id")]
        (is (= HydraMsg/GET_POST_OK
               (.id response)))
        (is (= (server/get-single-post dummy-backend "dummy-post-id")
               [(.post_id response)
                (.reply_to response)
                (.previous response)
                (.tags response)
                (.timestamp response)
                (.type response)
                (.content response)]))
        (is (= :connected
               (-> @(:state srv) vals first))))
      (finally
        (teardown cl-sock srv-sock)))))

(deftest test-invalid
  (let [[cl-sock srv-sock srv] (setup)]
    (try
      (let [response (server-client-comm cl-sock srv srv-sock msg/goodbye)]
        (is (= HydraMsg/INVALID
               (.id response)))
        (is (empty? @(:state srv))))
      (finally
        (teardown cl-sock srv-sock)))))

(deftest test-goodbye
  (let [[cl-sock srv-sock srv] (setup :connected)]
    (try
      (let [response (server-client-comm cl-sock srv srv-sock msg/goodbye)]
        (is (= HydraMsg/GOODBYE_OK
               (.id response)))
        (is (empty? @(:state srv))))
      (finally
        (teardown cl-sock srv-sock)))))

(deftest multiple-clients
  (let [srv-sock (msg/server-socket test-endpoint)
        srv (server/->Server srv-sock (atom {}) dummy-backend)
        client-1 (msg/client-socket test-endpoint)
        client-2 (msg/client-socket test-endpoint)]
    (try
      (let [r1 (server-client-comm client-1 srv srv-sock msg/hello)
            r2 (server-client-comm client-2 srv srv-sock msg/hello)]
        (is (= 2 (count @(:state srv))))
        (is (= [:connected :connected]
               (-> @(:state srv) vals)))
        (server-client-comm client-1 srv srv-sock msg/goodbye)
        (is (= [:connected]
               (vals @(:state srv)))))
      (finally
        (teardown client-1 srv-sock)
        (.close (:socket client-2))))))
