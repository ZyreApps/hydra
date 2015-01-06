(ns org.zproto.hydra-proto-test
  (:require [clojure.test :refer :all])
  (:import [org.zproto TestHydraProto]))

(deftest test-hydra-proto
  (is (nil? (.testHydraProto (TestHydraProto.)))))
