// Copyright 2023 Evolutionary Dynamics @ Harvard. All rights reserved.

#include <eulerian/generate.h>

#include <algorithm>
#include <climits>
#include <vector>

#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

namespace eulerian {

bool IsEulerian(const DiGraph& graph) {
  for (const auto vertex : boost::make_iterator_range(boost::vertices(graph))) {
    if (boost::in_degree(vertex, graph) != boost::out_degree(vertex, graph)) {
      return false;
    }
  }

  return true;
}

Generate::Generate(size_t N) : N_{N} {}

Generate::const_iterator Generate::begin() const { return const_iterator{N_, false}; }

Generate::const_iterator Generate::end() const { return const_iterator{N_, true}; }

Generate::const_iterator::const_iterator(size_t N, bool done) : N_{N}, done_{done}, graph_{N}, acc_graph_{N} {
  if (!done) {
    call_stack_.push_back(new var{0, 0, 0, N_, EdgeState::TRIED_NOTHING});
  }
  ++(*this);
}

bool IsWeaklyConnected(const DiGraph& graph) {
  UndirectedGraph undirected_graph{boost::num_vertices(graph)};
  for (const auto edge : boost::make_iterator_range(boost::edges(graph))) {
    boost::add_edge(edge.m_source, edge.m_target, undirected_graph);
  }

  std::vector<size_t> component{boost::num_vertices(undirected_graph)};
  const size_t num_components = boost::connected_components (undirected_graph, component.data());
  return num_components == 1;
}

// This algorithm is a bit complicated. Since C++ does not have a yield
// statement in the way that Python does, a `yield` functionality needed to be
// made, which involves storing states in a stack. The variable `call_stack_`
// really only holds one element at a time, but each element stores a pointer
// to its parent.
// For more info, see the following Python code:
// https://github.com/harvard-evolutionary-dynamics/Newell-Littlewood-Coefficient/blob/david/partitionsin.py
bool Generate::const_iterator::Next() {
  while (!call_stack_.empty()) {
    var* last_frame = call_stack_.back();

    if (last_frame->i == N_ && last_frame->j == 0) {
      if (IsEulerian(acc_graph_) && IsWeaklyConnected(acc_graph_)) {
        graph_ = acc_graph_;
        return true;
      }

      call_stack_.pop_back();
      delete last_frame;
      continue;
    }

    // Over-correction;
    if (last_frame->j == N_) {
      last_frame->j = 0;
      ++last_frame->i;
      last_frame->last_deg = last_frame->acc_deg;
      last_frame->acc_deg = 0;
      continue;
    }

    // The recursive calls.
    switch (last_frame->edge_state) {
      case EdgeState::TRIED_NOTHING: {
        // Try no edge.
        last_frame->edge_state = EdgeState::TRIED_NO_EDGE;
        call_stack_.push_back(new var{
            last_frame->i,
            last_frame->j + 1,
            last_frame->acc_deg,
            last_frame->last_deg,
            EdgeState::TRIED_NOTHING,
        });
        break;
      }

      case EdgeState::TRIED_NO_EDGE: {
        // Try adding an edge.
        last_frame->edge_state = EdgeState::TRIED_ADDING_EDGE;
        if (last_frame->acc_deg < last_frame->last_deg) {
          boost::add_edge(last_frame->i, last_frame->j, acc_graph_);
          call_stack_.push_back(new var{
              last_frame->i,
              last_frame->j + 1,
              last_frame->acc_deg + 1,
              last_frame->last_deg,
              EdgeState::TRIED_NOTHING,
          });
        }
        break;
      }

      case EdgeState::TRIED_ADDING_EDGE: {
        // We are done with this frame; cleanup time.
        boost::remove_edge(last_frame->i, last_frame->j, acc_graph_);
        call_stack_.pop_back();
        delete last_frame;
        break;
      }
    }
  }

  return false;
}

Generate::const_iterator& Generate::const_iterator::operator++() {
  if (!Next()) {
    done_ = true;
  }
  return *this;
}

const DiGraph& Generate::const_iterator::operator*() const { return acc_graph_; }

// Not totally correct but will do.
bool Generate::const_iterator::operator!=(
    const eulerian::Generate::const_iterator& rhs) const {
  return !done_ || !rhs.done_;
}

bool Generate::const_iterator::operator==(
    const eulerian::Generate::const_iterator& rhs) const {
  return !(*this != rhs);
}

}
// namespace eulerian
