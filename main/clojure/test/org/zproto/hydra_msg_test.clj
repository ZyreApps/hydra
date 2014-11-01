(ns org.zproto.hydra-msg-test
  (:require [clojure.test :refer :all])
  (:import [org.zproto TestHydraMsg]))

(deftest test-hydra-msg
  (is (nil? (.testHydraMsg (TestHydraMsg.)))))
