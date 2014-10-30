(defproject hydra-clj "0.1.0-SNAPSHOT"
  :description "FIXME: write description"
  :url "http://example.com/FIXME"
  :license {:name "Eclipse Public License"
            :url "http://www.eclipse.org/legal/epl-v10.html"}
  :java-source-paths ["main/java"]
  :source-paths ["main/clojure"]
  :prep-tasks ["javac"]
  :dependencies [[org.clojure/clojure "1.6.0"]
                 [junit/junit "4.11"]
                 [org.zeromq/jeromq "0.3.4"]
                 [org.zeromq/cljzmq "0.1.4" :exclude [jzmq]]])
