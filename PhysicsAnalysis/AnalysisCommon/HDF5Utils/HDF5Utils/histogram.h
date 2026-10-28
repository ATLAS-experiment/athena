/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef H5_HISTOGRAMS_H
#define H5_HISTOGRAMS_H

namespace H5 {
  class Group;
}

#include <string>

// the main user-facing functions are defined as members of a struct
// rather than free functions to ensure that the full set of overloads
// here matches those in the `.icc` file. The struct has one global
// instance, `write_hist_to_group`, that behaves exactly like a free
// function.


namespace H5Utils::hist::detail {
  struct write_hist_to_group_fn {
    // Simple form, writes boost::histogram to an HDF5 group with some
    // name.
    template <typename T>
    void operator()(
      H5::Group& group,
      const T& hist,
      const std::string& name) const;

    // Overload with external label override (positional list or name
    // map). By default axes are labeled in the boost::histogram
    // metadata, but this can be overwritten when writing. Several
    // forms for `Labels` are allowed:
    //
    // - A mapping from axis name to axis labeling overrides. All keys
    //   must be valid axis names in the existing histogram, but any
    //   axes without an override are left unchanged.
    //
    // - A list of axis labeling overrides. This list must be equal in
    //  length to the number of axes in the histogram.
    //
    // The axis labeling itself specifies bin names in discrete axes,
    // and can either be a list of labels (in the same order in which
    // bins were specified) or a mapping between the discrete bin
    // values and the label names. For more examples see
    // `util/test-histogram.cxx`.
    //
    template <typename T, typename Labels>
    void operator()(
      H5::Group& group,
      const T& hist,
      const std::string& name,
      const Labels& labels) const;
  };
}

namespace H5Utils::hist {
  inline constexpr detail::write_hist_to_group_fn write_hist_to_group{};
}

#include "histogram.icc"

#endif
