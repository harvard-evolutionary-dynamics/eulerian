// Copyright (c) 2023 Evolutionary Dynamics @ Harvard. All rights reserved.

#ifndef EULERIAN_GENERATE_H_
#define EULERIAN_GENERATE_H_

#include <cstdint>
#include <iterator>
#include <vector>

#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>

namespace eulerian {

using DiGraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS>;
using UndirectedGraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;


bool IsEulerian(const DiGraph& graph);
bool IsWeaklyConnected(const DiGraph& graph);

// Iterates through each connected Eulerian digraph with N nodes.
class Generate {
 public:
  Generate(size_t N);

  class const_iterator : public std::iterator<std::forward_iterator_tag, DiGraph> {
   public:
    const_iterator(size_t N, bool done);
    const_iterator& operator++();
    const DiGraph& operator*() const;
    bool operator!=(const const_iterator&) const;
    bool operator==(const const_iterator&) const;

   private:
    enum class EdgeState { TRIED_NOTHING, TRIED_NO_EDGE, TRIED_ADDING_EDGE };
    struct var {
      size_t i;
      size_t j;
      size_t acc_deg;
      size_t last_deg;
      EdgeState edge_state;
      // var* came_from;

      var(size_t i, size_t j, size_t acc_deg, size_t last_deg,
          EdgeState edge_state)
          : i{i}, j{j}, acc_deg{acc_deg}, last_deg{last_deg}, edge_state{edge_state} {}

      // var(size_t level, int64_t rem, int64_t mn, int64_t mx, var* came_from)
      //     : level(level), rem(rem), mn(mn), mx(mx), came_from(came_from) {}
    };

    bool Next();
    // bool GoBack(var* v);

   private:
    const size_t N_;
    bool done_;
    DiGraph graph_;
    DiGraph acc_graph_;
    std::vector<var*> call_stack_;
  };

 public:
  const_iterator begin() const;
  const_iterator end() const;

 private:
  const size_t N_;
};

// Computes the intersection of two partitions.
// const Partition Intersection(const Partition&, const Partition&);

}  // namespace eulerian

#endif  // EULERIAN_GENERATE_H_
