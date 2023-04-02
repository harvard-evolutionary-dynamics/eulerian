// Copyright (c) 2020 [Your Name]. All rights reserved.

#define CATCH_CONFIG_MAIN

#include <exception>
#include <iostream>
#include <numeric>

#include <catch2/catch.hpp>

#include <eulerian/generate.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/config.hpp>

TEST_CASE("Empty graph is not weakly connected", "[IsWeaklyConnected]") {
  eulerian::DiGraph graph{3};
  REQUIRE_FALSE(eulerian::IsWeaklyConnected(graph));
}

TEST_CASE("0 <- 1 -> 2 is weakly connected", "[IsWeaklyConnected]") {
  eulerian::DiGraph graph{3};
  boost::add_edge(1, 0, graph);
  boost::add_edge(1, 2, graph);
  REQUIRE(eulerian::IsWeaklyConnected(graph));
}

TEST_CASE("0 <- 1 -> 2  . 3 is not weakly connected", "[IsWeaklyConnected]") {
  eulerian::DiGraph graph{4};
  boost::add_edge(1, 0, graph);
  boost::add_edge(1, 2, graph);
  REQUIRE_FALSE(eulerian::IsWeaklyConnected(graph));
}

TEST_CASE("Empty graph is Eulerian", "[IsEulerian]") {
  eulerian::DiGraph graph{3};
  REQUIRE(eulerian::IsEulerian(graph));
}

TEST_CASE("0 <- 1 -> 2 is not Eulerian", "[IsEulerian]") {
  eulerian::DiGraph graph{3};
  boost::add_edge(1, 0, graph);
  boost::add_edge(1, 2, graph);
  REQUIRE_FALSE(eulerian::IsEulerian(graph));
}

TEST_CASE("3-cycle is Eulerian", "[IsEulerian]") {
  eulerian::DiGraph graph{3};
  boost::add_edge(0, 1, graph);
  boost::add_edge(1, 2, graph);
  boost::add_edge(2, 0, graph);
  REQUIRE(eulerian::IsEulerian(graph));
}
