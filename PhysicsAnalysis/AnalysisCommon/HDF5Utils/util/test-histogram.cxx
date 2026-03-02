/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "HDF5Utils/histogram.h"

#include "H5Cpp.h"

int main(int, char*[]) {

  namespace bh = boost::histogram;
  namespace h5h = H5Utils::hist;
  using def = bh::use_default;

  using dax_t = bh::axis::regular<double>;
  using daxn_t = bh::axis::regular<double, def, def, bh::axis::option::none_t>;
  using iax_t = bh::axis::integer<int>;
  using cax_t = bh::axis::category<short, def, bh::axis::option::overflow_t>;

  H5::H5File out_file("hists.h5", H5F_ACC_TRUNC);

  // build weighted hist
  {
    auto h3dw = bh::make_weighted_histogram(
      daxn_t(1, -0.5, 0.5, "ax0"),
      dax_t(1, -0.5, 0.5, "ax1"),
      iax_t(-1, 1, "ax2"));
    // this should put a 0.5 at [0, 1, 2] when all the overflows are
    // accounted for
    h3dw(0, 0, 0, bh::weight(0.5));
    // this is to test the overflow bin
    h3dw(0, 0, 20, bh::weight(10));
    h5h::write_hist_to_group(out_file, h3dw, "h3dw");
  }

  // build unweighted hist
  {
    auto h3d = bh::make_histogram(
      daxn_t(1, -0.5, 0.5, "ax0"),
      dax_t(1, -0.5, 0.5, "ax1"),
      iax_t(-1, 1, "ax2"));
    h3d(0, 0, 0);
    h5h::write_hist_to_group(out_file, h3d, "h3d");
  }

  // build weighted profile hist
  {
    auto h3dp = bh::make_weighted_profile(
      daxn_t(1, -0.5, 0.5, "ax0"),
      dax_t(1, -0.5, 0.5, "ax1"),
      iax_t(-1, 1, "ax2"));
    // we need two calls here to check the variance too
    h3dp(0, 0, 0, bh::weight(0.5), bh::sample(1.0));
    h3dp(0, 0, 0, bh::weight(0.5), bh::sample(1.0));
    h5h::write_hist_to_group(out_file, h3dp, "h3dp");
  }

  // build a dyanmic hist
  {
    using variant = bh::axis::variant<dax_t, daxn_t, iax_t>;
    std::vector<variant> axes {
      daxn_t(1, -0.5, 0.5, "ax0"),
      dax_t(1, -0.5, 0.5, "ax1"),
      iax_t(-1, 1, "ax2")
    };
    auto hdyn = bh::make_weighted_histogram(axes);
    // seems we need to do some funny organization to make weighting
    // work with dynamic axes.
    std::vector<std::vector<double>> vals { {0}, {0}, {0} };
    hdyn.fill(vals, bh::weight(0.5));
    h5h::write_hist_to_group(out_file, hdyn, "hdyn");
  }

  // integer storage
  {
    using int_storage = bh::dense_storage<int64_t>;
    auto h1i = bh::make_histogram_with(
      int_storage{},
      dax_t(3, -1.5, 1.5, "x"));
    h1i(-1.0);  // bin 0
    h1i(0.0);   // bin 1
    h1i(0.0);   // bin 1 again
    h5h::write_hist_to_group(out_file, h1i, "h1i");
  }

  // thread-safe (atomic) integer storage
  {
    using atomic_int_storage =
      bh::dense_storage<bh::accumulators::count<int64_t, true>>;
    auto h1a = bh::make_histogram_with(
      atomic_int_storage{},
      dax_t(3, -1.5, 1.5, "x"));
    h1a(0.0);   // bin 1
    h1a(1.0);   // bin 2
    h5h::write_hist_to_group(out_file, h1a, "h1a");
  }

  // thread-safe double storage
  {
    using atomic_double_storage =
      bh::dense_storage<bh::accumulators::count<double, true>>;
    auto h1td = bh::make_histogram_with(
      atomic_double_storage{},
      dax_t(3, -1.5, 1.5, "x"));
    h1td(-1.0);  // bin 0
    h1td(0.0);   // bin 1
    h1td(0.0);   // bin 1 again
    h5h::write_hist_to_group(out_file, h1td, "h1td");
  }

  // thread-safe float storage
  {
    using atomic_float_storage =
      bh::dense_storage<bh::accumulators::count<float, true>>;
    auto h1tf = bh::make_histogram_with(
      atomic_float_storage{},
      dax_t(3, -1.5, 1.5, "x"));
    h1tf(0.0);   // bin 1
    h1tf(1.0);   // bin 2
    h1tf(1.0);   // bin 2 again
    h5h::write_hist_to_group(out_file, h1tf, "h1tf");
  }

  // categorical axes test
  {
    // flavor labels histogram
    auto h1c = bh::make_histogram(
      cax_t({0, 4, 5, 15}, "flavorTruthLabel") );
    h1c(5);                     // add a b-hadron
    h1c(20);                   // no idea what, test overflow
    h5h::write_hist_to_group(out_file, h1c, "h1c");
  }


  return 0;
}
