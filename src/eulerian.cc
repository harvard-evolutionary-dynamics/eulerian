// Copyright 2020 ICLUE @ UIUC. All rights reserved.

#include <algorithm>
#include <climits>
#include <cstdint>
#include <exception>
#include <map>
#include <numeric>
#include <vector>

#include <omp.h>

extern "C" {
#include <lrcalc/hashtab.h>
#include <lrcalc/symfcn.h>
#include <lrcalc/vector.h>
}

#include <eulerian/eulerian.h>
#include <eulerian/partitions_in.h>

namespace eulerian {

vector* to_vector(const Partition& vec) {
  vector* v = v_new(static_cast<int32_t>(vec.size()));
  for (size_t i = 0; i < vec.size(); ++i) {
    v->array[i] = static_cast<int32_t>(vec[i]);
  }
  return v;
}

bool to_cppvec(const vector* v, Partition* vec) {
  if (v == nullptr || vec == nullptr) return false;
  vec->clear();

  const size_t n = v->length;
  vec->reserve(n);

  for (size_t i = 0; i < v->length; ++i) {
    // We know that everything we are working with will be non-negative.
    vec->push_back(static_cast<NonNegInt>(v->array[i]));
  }

  return true;
}

int64_t lrcoef(const Partition& outer, const Partition& inner1,
               const Partition& inner2) {
  ValidatePartitions({outer, inner1, inner2});

  vector* o = to_vector(outer);
  vector* i1 = to_vector(inner1);
  vector* i2 = to_vector(inner2);

  const int64_t result = lrcoef(o, i1, i2);

  v_free(o);
  v_free(i1);
  v_free(i2);

  return result;
}

// Evaluates one term in the sum.
int64_t nlcoef_slow_helper(const Partition& alpha, const Partition& mu,
                           const Partition& nu, const Partition& lambda) {
  vector* aa = to_vector(alpha);
  vector* mm = to_vector(mu);
  vector* nn = to_vector(nu);
  hashtab* s1 = skew(mm, aa, 0);
  hashtab* s2 = skew(nn, aa, 0);
  v_free(aa);
  v_free(mm);
  v_free(nn);

  Coefficients ss1, ss2;
  to_map(s1, &ss1);
  to_map(s2, &ss2);
  hash_free(s1);
  hash_free(s2);

  int64_t nl_im = 0;
  for (const auto& p1 : ss1) {
    for (const auto& p2 : ss2) {
      // These are the 2 LR coefficients.
      const auto c1 = p1.second;
      const auto c2 = p2.second;

      vector* v1 = to_vector(p1.first);
      vector* v2 = to_vector(p2.first);
      hashtab* ht = mult(v1, v2, 0);
      v_free(v1);
      v_free(v2);

      Coefficients ss;
      to_map(ht, &ss);
      hash_free(ht);

      if (ss.find(lambda) != ss.end()) {
        const auto c3 = ss[lambda];
        nl_im += c1 * c2 * c3;
      }
    }
  }

  return nl_im;
}

NonNegInt Sum(const Partition& parts) {
  return static_cast<NonNegInt>(
      std::accumulate(parts.begin(), parts.end(), 0ul));
}

int64_t nlcoef_slow(const Partition& mu, const Partition& nu,
                    const Partition& lambda) {
  ValidatePartitions({mu, nu, lambda});

  int64_t nl = 0;
  // Step 1: Compute the intersection of mu and nu.
  const Partition limit = Intersection(mu, nu);
  const size_t slimit = Sum(limit);

  // Step 2: Iterate through each partition in the intersection.
  for (size_t size = 0; size <= slimit; ++size) {
    for (const Partition& alpha : PartitionsIn(limit, size)) {
      nl += nlcoef_slow_helper(alpha, mu, nu, lambda);
    }
  }

  return nl;
}

bool to_map(hashtab* ht, Coefficients* m) {
  if (ht == nullptr || m == nullptr) return false;
  m->clear();

  hash_itr itr;
  hash_first(ht, itr);
  while (hash_good(itr)) {
    const vector* v = static_cast<vector*>(hash_key(itr));
    Partition p;
    to_cppvec(v, &p);
    const int32_t c = hash_intvalue(itr);
    m->insert({p, c});
    hash_next(itr);
  }

  return true;
}

bool NeedsComputation(const Partition& mu, const Partition& nu,
                      const Partition& lambda, int64_t* nl,
                      std::vector<Partition>* va, std::vector<Partition>* vb,
                      std::vector<Partition>* vc) {
  ValidatePartitions({mu, nu, lambda});
  if (nl == nullptr) return true;

  const Partition int_mn = Intersection(mu, nu);
  const Partition int_ml = Intersection(mu, lambda);
  const Partition int_nl = Intersection(nu, lambda);

  const size_t sl = Sum(lambda);
  const size_t sm = Sum(mu);
  const size_t sn = Sum(nu);

  // Lemma 2.2 (v).
  if ((sl + sm + sn) % 2 == 1) {
    *nl = 0;
    return false;
  }

  // Lemma 2.2 (iii).
  if (sl + sm < sn || sm + sn < sl || sl + sn < sm) {
    *nl = 0;
    return false;
  }

  // Lemma 2.2 (ii).
  // TODO: permutation invariant, so add other permutations as well.
  if (sm + sn == sl) {
    *nl = lrcoef(lambda, mu, nu);
    return false;
  }

  // Lemma 2.2 Equation 6.
  const size_t sa = ((sm + sn) - sl) / 2;
  const size_t sb = ((sl + sm) - sn) / 2;
  const size_t sc = ((sl + sn) - sm) / 2;

  const auto vfn = [&](const PartitionsIn&& pi) {
    std::vector<Partition> v;
    for (const Partition& partition : pi) {
      v.push_back(partition);
    }
    return v;
  };

  if (va == nullptr || vb == nullptr || vc == nullptr) {
    *nl = 0;
    return false;
  }

  *va = vfn(PartitionsIn(int_mn, sa));
  *vb = vfn(PartitionsIn(int_ml, sb));
  *vc = vfn(PartitionsIn(int_nl, sc));

  return true;
}

int64_t nlcoef(const Partition& mu, const Partition& nu,
               const Partition& lambda) {
  return nlcoef(mu, nu, lambda, false);
}

int64_t nlcoef(const Partition& mu, const Partition& nu,
               const Partition& lambda, const bool check_positivity) {
  int64_t nl;
  std::vector<Partition> va;
  std::vector<Partition> vb;
  std::vector<Partition> vc;

  if (!NeedsComputation(mu, nu, lambda, &nl, &va, &vb, &vc)) {
    return check_positivity ? static_cast<int64_t>(nl > 0) : nl;
  }

  bool is_positive = false;
  nl = 0;
  // The `Release` version seems to benefit from OpenMP parallel directives.
  // Use a `dynamic` schedule since the work is not balanced among iterations.
#pragma omp parallel
  {
#pragma omp for reduction(+ : nl) schedule(dynamic)
    for (auto ita = va.begin(); ita < va.end(); ++ita) {
      const Partition& alpha = *ita;
      for (auto itb = vb.begin(); itb < vb.end(); ++itb) {
        const Partition& beta = *itb;
        const int64_t cabm = lrcoef(mu, alpha, beta);
        if (cabm == 0) continue;
        for (auto itc = vc.begin(); itc < vc.end(); ++itc) {
          const Partition& gamma = *itc;

          const int64_t cacn = lrcoef(nu, alpha, gamma);
          if (cacn == 0) continue;
          const int64_t cbcl = lrcoef(lambda, beta, gamma);
          if (cbcl == 0) continue;

          if (check_positivity) {
#ifndef _OPENMP
            return 1;
#endif
#pragma omp atomic write
            is_positive = true;
#pragma omp cancel for
          }

          // Add to the answer.
          nl += cabm * cacn * cbcl;
        }
      }

      if (check_positivity) {
#pragma omp cancellation point for
      }
    }
  }

  return check_positivity ? static_cast<int64_t>(is_positive) : nl;
}

Coefficients skew(const Partition& outer, const Partition& inner) {
  return skew(outer, inner, 0);
}

Coefficients skew(const Partition& outer, const Partition& inner,
                  const size_t max_rows) {
  ValidatePartitions({outer, inner});

  vector* v1 = to_vector(outer);
  vector* v2 = to_vector(inner);
  hashtab* ht = skew(v1, v2, static_cast<int32_t>(max_rows));

  Coefficients coefficients;
  to_map(ht, &coefficients);

  v_free(v1);
  v_free(v2);
  hash_free(ht);

  return coefficients;
}

}  // namespace eulerian

