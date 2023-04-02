// Copyright (c) 2020 ICLUE @ UIUC. All rights reserved.

#ifndef EULERIAN_PARTITIONS_IN_H_
#define EULERIAN_PARTITIONS_IN_H_

#include <cstdint>
#include <iterator>
#include <vector>

namespace eulerian {

typedef uint64_t NonNegInt;
typedef std::vector<uint64_t> Partition;

void ValidatePartitions(const std::vector<Partition>& partitions);

// Iterates through each partition contained within a given partition `limit`.
class PartitionsIn {
 public:
  PartitionsIn(const Partition& limit, size_t size);

  class const_iterator : public std::iterator<std::forward_iterator_tag, Partition> {
   public:
    const_iterator(const Partition& limit, size_t size);
    const_iterator& operator++();
    const Partition& operator*() const;
    bool operator!=(const const_iterator&) const;
    bool operator==(const const_iterator&) const;

   private:
    struct var {
      size_t level;
      int64_t rem;
      int64_t mn;
      int64_t mx;
      var* came_from;

      var(size_t level, int64_t rem, int64_t mn, int64_t mx, var* came_from)
          : level(level), rem(rem), mn(mn), mx(mx), came_from(came_from) {}
    };

    bool Next();
    bool GoBack(var* v);

   private:
    const Partition limit_;
    const size_t size_;
    Partition parts_;
    Partition ret_parts_;
    std::vector<Partition::value_type> rsums_;
    std::vector<var*> call_stack_;
    bool done_;
  };

 public:
  const_iterator begin() const;
  const_iterator end() const;

 private:
  const size_t size_;
  const Partition limit_;
};

// Computes the intersection of two partitions.
const Partition Intersection(const Partition&, const Partition&);

}  // namespace eulerian

#endif  // EULERIAN_PARTITIONS_IN_H_
