// Copyright 2020 ICLUE @ UIUC. All rights reserved.

#include <algorithm>
#include <climits>
#include <vector>

#include <eulerian/partitions_in.h>

namespace eulerian {

// Make sure partitions are weakly decreasing and only positive parts.
void ValidatePartitions(const std::vector<Partition>& partitions) {
  for (const Partition& partition : partitions) {
    NonNegInt last = UINT64_MAX;
    for (const auto part : partition) {
      if (last < part) {
        throw std::invalid_argument(
            "Each partition must be weakly decreasing.");
      }

      last = part;
    }
  }
}

// The variable `limit` does not actually need to be a partition and won't
// be checked for such.
PartitionsIn::PartitionsIn(const Partition& limit, const size_t size)
    : size_{size}, limit_{limit} {}

PartitionsIn::const_iterator PartitionsIn::begin() const {
  return const_iterator{limit_, size_};
}

PartitionsIn::const_iterator PartitionsIn::end() const {
  return const_iterator{{}, 0};
}

PartitionsIn::const_iterator::const_iterator(const Partition& limit,
                                             const size_t size)
    : limit_{limit}, size_{size}, done_{false} {
  if (limit.empty()) {
    done_ = true;
    return;
  }

  // Keep track of suffix sums.
  for (auto it = limit_.rbegin(); it != limit_.rend(); ++it) {
    const NonNegInt last = !rsums_.empty() ? rsums_.back() : 0;
    rsums_.push_back(last + *it);
  }
  std::reverse(rsums_.begin(), rsums_.end());

  call_stack_.push_back(
      new var{0, static_cast<int64_t>(size_), 1, -1, nullptr});

  ++(*this);
}

bool PartitionsIn::const_iterator::GoBack(var* v) {
  var* came_from = v->came_from;
  delete v;

  if (came_from == nullptr) return false;
  parts_.pop_back();
  ++came_from->mn;
  call_stack_.push_back(came_from);
  return true;
}

// This algorithm is a bit complicated. Since C++ does not have a yield
// statement in the way that Python does, a `yield` functionality needed to be
// made, which involves storing states in a stack. The variable `call_stack_`
// really only holds one element at a time, but each element stores a pointer
// to its parent.
// For more info, see the following Python code:
// https://github.com/harvard-evolutionary-dynamics/Newell-Littlewood-Coefficient/blob/david/partitionsin.py
bool PartitionsIn::const_iterator::Next() {
  // Handle the edge case where the size is zero. This is valid.
  if (size_ == 0) {
    if (call_stack_.empty()) return false;
    ret_parts_ = {0};
    var* v = call_stack_.back();
    call_stack_.pop_back();
    delete v;
    return true;
  }

  while (!call_stack_.empty()) {
    var* v = call_stack_.back();
    call_stack_.pop_back();

    const int64_t left = static_cast<int64_t>(limit_.size()) -
                         static_cast<int64_t>(parts_.size());

    if (v->rem == 0 && parts_.size() <= limit_.size()) {
      ret_parts_ = parts_;
      if (!GoBack(v)) break;
      return true;
    } else if (v->rem < 0 || v->level >= limit_.size() ||
               static_cast<int64_t>(rsums_[v->level]) < v->rem ||
               (!parts_.empty() &&
                static_cast<int64_t>(parts_.back()) * left < v->rem)) {
      if (!GoBack(v)) break;
    } else if (v->mx == -1) {
      const auto max_part = std::min(
          v->rem,
          static_cast<int64_t>(std::min(
              limit_[v->level], !parts_.empty() ? parts_.back() : size_)));
      v->mx = max_part;
      if (v->mn <= v->mx) {
        parts_.push_back(1);
        call_stack_.push_back(new var{v->level + 1, v->rem - 1, 1, -1, v});
      } else if (!GoBack(v)) {
        break;
      }
    } else if (v->mn <= v->mx) {
      parts_.push_back(static_cast<NonNegInt>(v->mn));
      call_stack_.push_back(new var{v->level + 1, v->rem - v->mn, 1, -1, v});
    } else if (!GoBack(v)) {
      break;
    }
  }

  return false;
}

PartitionsIn::const_iterator& PartitionsIn::const_iterator::operator++() {
  if (!Next()) {
    done_ = true;
  }
  return *this;
}

const Partition& PartitionsIn::const_iterator::operator*() const {
  return ret_parts_;
}

bool PartitionsIn::const_iterator::operator!=(
    const eulerian::PartitionsIn::const_iterator& rhs) const {
  return !done_ || !rhs.done_;
}

bool PartitionsIn::const_iterator::operator==(
const eulerian::PartitionsIn::const_iterator& rhs) const {
  return !(*this != rhs);
}

// Computes the intersection of two partitions.
const Partition Intersection(const Partition& a, const Partition& b) {
  const size_t n = std::min(a.size(), b.size());
  Partition c(n);
  for (size_t i = 0; i < n; ++i) {
    c[i] = std::min(a[i], b[i]);
  }

  return c;
}

}  // namespace eulerian
