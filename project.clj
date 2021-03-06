;;  =========================================================================
;;    Hydra Server (in C)
;;
;;    ** WARNING *************************************************************
;;    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
;;    your changes at the next build cycle. This is great for temporary printf
;;    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
;;    for commits are:
;;
;;    * The XML model used for this code generation: hydra_server.xml
;;    * The code generation script that built this file: zproto_server_c
;;    ************************************************************************
;;    =========================================================================
(defproject hydra-server "0.1.0-SNAPSHOT"
  :description "FIXME: write description"
  :url "http://example.com/FIXME"
  :java-source-paths ["main/java"]
  :source-paths ["main/clojure/src"]
  :test-paths ["main/clojure/test"]
  :prep-tasks ["javac"]
  :dependencies [[org.clojure/clojure "1.6.0"]
                 [junit/junit "4.11"]
                 [org.zeromq/cljzmq "0.1.4" :exclusions [jzmq]]
                 [org.zeromq/jeromq "0.3.4"]]
  :main org.zproto.hydra-server-impl)
