// Copyright 2020 ICLUE @ UIUC. All rights reserved.

#include <cstdint>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <eulerian/eulerian.h>

namespace py = pybind11;

PYBIND11_MODULE(eulerian, m) {
  m.doc() = R"pbdoc(
    Pybind11 example plugin
    -----------------------
    .. currentmodule:: cmake_example
    .. autosummary::
       :toctree: _generate
       lrcoef
       nlcoef
       nlcoef_slow
    )pbdoc";

  m.def("nlcoef_slow", &eulerian::nlcoef_slow, R"pbdoc(
    Compute a single Newell-Littlewood coefficient using Proposition 2.3.
    INPUT:
    - ``mu`` -- a partition (weakly decreasing list of non-negative integers).
    - ``nu`` -- a partition.
    - ``lambda`` -- a partition.
    EXAMPLES::
        python: from eulerian import nlcoef_slow
        python: nlcoef_slow([2,1], [2,1], [4, 2])
        0
  )pbdoc");

  m.def("skew",
        [](const eulerian::Partition& outer, const eulerian::Partition& inner,
           const size_t max_rows) {
          const auto coefficients = eulerian::skew(outer, inner, max_rows);

          // Python lists are not hashable, so since std::vectors are implicitly
          // cast to Python lists, we need to instead explicitly cast them to
          // Python tuples.
          py::dict d;
          for (const auto& e : coefficients) {
            const py::tuple tuple = py::cast(e.first);
            d[tuple] = e.second;
          }

          return d;
        },
        R"pbdoc(
    Compute the Schur expansion of a skew Schur function.

    Return a linear combination of partitions representing the Schur
    function of the skew Young diagram ``outer / inner``, consisting
    of boxes in the partition ``outer`` that are not in ``inner``.
    INPUT:
    - ``outer`` -- a partition.
    - ``inner`` -- a partition.
    - ``maxrows`` -- an integer or None.
    If ``maxrows`` is specified, then only partitions with at most
    this number of rows are included in the result.
    EXAMPLES::
        sage: from sage.libs.lrcalc.lrcalc import skew  #optional - lrcalc
        sage: sorted(skew([2,1],[1]).items())           #optional - lrcalc
        [([1, 1], 1), ([2], 1)]
    )pbdoc",
        py::arg("outer"), py::arg("inner"), py::arg("max_rows") = 0);

  m.def(
      "nlcoef",
      py::overload_cast<const eulerian::Partition&, const eulerian::Partition&,
                          const eulerian::Partition&, bool>(&eulerian::nlcoef),
        R"pbdoc(
    Compute a single Newell-Littlewood coefficient using the definition (1.1).
    INPUT:
    - ``mu`` -- a partition (weakly decreasing list of non-negative integers).
    - ``nu`` -- a partition.
    - ``lambda`` -- a partition.
    EXAMPLES::
        python: from eulerian import nlcoef
        python: nlcoef([8, 4, 4], [8, 4, 4], [8, 4, 4])
        141
        python: nlcoef([8, 4, 4], [8, 4, 4], [8, 4, 4], check_positivity=True)
        1
        python: nlcoef([2, 1], [2, 1], [4, 2])
        0
        python: nlcoef([2, 1], [2, 1], [4, 2], check_positivity=True)
        0
    NOTES::
        If you want check_positivity to be heavily optimized, the environment
        variable `OMP_CANCELLATION` needs to be set to `true` BEFORE importing
        this module.
  )pbdoc",
  py::arg("mu"), py::arg("nu"), py::arg("lambda"), py::arg("check_positivity") = false);

  m.def("lrcoef", &eulerian::lrcoef, R"pbdoc(
    Compute a single Littlewood-Richardson coefficient.
    Return the coefficient of ``outer`` in the product of the Schur
    functions indexed by ``inner1`` and ``inner2``.
    INPUT:
    - ``outer`` -- a partition (weakly decreasing list of non-negative integers).
    - ``inner1`` -- a partition.
    - ``inner2`` -- a partition.
    EXAMPLES::
        python: from eulerian import lrcoef
        python: lrcoef([3,2,1], [2,1], [2,1])
        2
        python: lrcoef([3,3], [2,1], [2,1])
        1
        python: lrcoef([2,1,1,1,1], [2,1], [2,1])
        0
  )pbdoc");

  py::class_<eulerian::PartitionsIn>(m, "PartitionsIn")
      .def(py::init<const eulerian::Partition&, const size_t>())
      // Keep this class alive while the returned iterator is alive.
      .def("__iter__", [](eulerian::PartitionsIn& p) {
        return py::make_iterator(p.begin(), p.end());
      }, py::keep_alive<0, 1>());

  #ifdef VERSION_INFO
  m.attr("__version__") = VERSION_INFO;
  #else
  m.attr("__version__") = "dev";
  #endif
}
