(ns org.zproto.hydra-server-test
  (:require [clojure.test :refer :all]
            [org.zproto.hydra-server :as server]
            [org.zproto.hydra-msg :as msg])
  (:import [org.zproto HydraMsg]))

(def test-endpoint "inproc://hydra-server-test")

(def dummy-backend
  (reify server/HydraServerBackend
    (get-latest-post [this m]
      (msg/post-id! m "dummy-post-id"))
    (get-single-post [this m post-id]
      (msg/reply-to! m "none")
      (msg/previous! m "none")
      (msg/tags! m "tag_2")
      (msg/timestamp! m 123123123)
      (msg/type! m "dummy")
      (msg/content! m "dummy-post"))
    (get-all-tags [this m]
      (msg/tags! m "tag_1 tag_2 tag_3 tag_4"))
    (get-single-tag [this m tag-id]
      (msg/post-id! m "dummy-post-id-2"))))

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

(defn blank-msg []
  (HydraMsg. 0))

(defn server-client-comm [cl-sock srv srv-sock cl-msg & args]
  (apply cl-msg cl-sock args)
  (server/match-msg srv (msg/recv srv-sock))
  (msg/recv cl-sock))

(deftest test-hello-ok
  (let [[cl-sock srv-sock srv] (setup)]
    (try
      (let [response (server-client-comm cl-sock srv srv-sock msg/hello)
            expected (blank-msg)]
        (server/get-latest-post dummy-backend expected)
        (is (= HydraMsg/HELLO_OK
               (.id response)))
        (is (= (.post_id expected)
               (.post_id response)))
        (is (= :connected
               (-> @(:state srv) vals first))))
      (finally
        (teardown cl-sock srv-sock)))))

(deftest test-get-tags-ok
  (let [[cl-sock srv-sock srv] (setup :connected)]
    (try
      (let [response (server-client-comm cl-sock srv srv-sock msg/get-tags)
            expected (blank-msg)]
        (server/get-all-tags dummy-backend expected)
        (is (= HydraMsg/GET_TAGS_OK
               (.id response)))
        (is (= (.tags expected)
               (.tags response)))
        (is (= :connected
               (-> @(:state srv) vals first))))
      (finally
        (teardown cl-sock srv-sock)))))

(deftest test-get-tag-ok
  (let [[cl-sock srv-sock srv] (setup :connected)]
    (try
      (let [tag "tag_1"
            response (server-client-comm cl-sock srv srv-sock msg/get-tag tag)
            expected (blank-msg)]
        (server/get-single-tag dummy-backend expected tag)
        (is (= HydraMsg/GET_TAG_OK
               (.id response)))
        (is (= tag
               (.tag response)))
        (is (= (.post_id expected)
               (.post_id response)))
        (is (= :connected
               (-> @(:state srv) vals first))))
      (finally
        (teardown cl-sock srv-sock)))))

(deftest test-get-post-ok
  (let [[cl-sock srv-sock srv] (setup :connected)]
    (try
      (let [post-id "dummy-post-id"
            response (server-client-comm cl-sock srv srv-sock msg/get-post post-id)
            expected (blank-msg)]
        (server/get-single-post dummy-backend expected post-id)
        (is (= HydraMsg/GET_POST_OK
               (.id response)))
        (is (= post-id
               (.post_id response)))
        (is (= (.reply_to expected)
               (.reply_to response)))
        (is (= (.previous expected)
               (.previous response)))
        (is (= (.tags expected)
               (.tags response)))
        (is (= (.timestamp expected)
               (.timestamp response)))
        (is (= (.type expected)
               (.type response)))
        (is (= (.content expected)
               (.content response)))
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
