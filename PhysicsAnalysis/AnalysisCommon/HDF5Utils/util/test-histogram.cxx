/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HDF5Utils/histogram.h"

#include "H5Cpp.h"
#include <deque>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

void run_tests() {

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

  // unsigned char (uint8) storage
  {
    using uint8_storage = bh::dense_storage<uint8_t>;
    auto h1u8 = bh::make_histogram_with(
      uint8_storage{}, dax_t(3, -1.5, 1.5, "x"));
    h1u8(0.0);   // bin 1, storage index 2
    h1u8(1.0);   // bin 2, storage index 3
    h5h::write_hist_to_group(out_file, h1u8, "h1u8");
  }

  // unsigned short (uint16) storage
  {
    using uint16_storage = bh::dense_storage<uint16_t>;
    auto h1u16 = bh::make_histogram_with(
      uint16_storage{}, dax_t(3, -1.5, 1.5, "x"));
    h1u16(0.0);   // bin 1, storage index 2
    h1u16(1.0);   // bin 2, storage index 3
    h5h::write_hist_to_group(out_file, h1u16, "h1u16");
  }

  // Labeled categorical axis: plain enum class (no explicit underlying type)
  {
    enum class Flavor { light, c, b, tau };
    using meta_t = std::pair<std::string, std::vector<std::string>>;
    using flavor_ax_t = bh::axis::category<Flavor, meta_t,
                                           bh::axis::option::overflow_t>;
    auto h_flavor = bh::make_histogram(
      flavor_ax_t(
        {Flavor::light, Flavor::c, Flavor::b, Flavor::tau},
        meta_t{"flavorLabel", {"light", "c", "b", "tau"}}
      )
    );
    h_flavor(Flavor::b);
    h_flavor(Flavor::c);
    h5h::write_hist_to_group(out_file, h_flavor, "h_flavor_labeled");
  }

  // Labeled 2-D histogram: one regular axis, one labeled categorical
  {
    using meta_t = std::pair<std::string, std::vector<std::string>>;
    using labeled_cax = bh::axis::category<int, meta_t>;
    auto h2d_cat = bh::make_weighted_histogram(
      dax_t(4, 0.0, 4.0, "pt_bin"),
      labeled_cax(
        {0, 1, 2},
        meta_t{"nTracks", {"zero", "one", "two"}}
      )
    );
    h2d_cat(1.5, 1, bh::weight(0.5));
    h5h::write_hist_to_group(out_file, h2d_cat, "h2d_cat");
  }

  // variable-width axis - edges stored as float64 (matching Python)
  {
    using vax_t = bh::axis::variable<double>;
    auto h1v = bh::make_histogram(
      vax_t({0.0, 1.0, 3.0, 10.0}, "pt"));
    h1v(0.5);   // bin 0  [0, 1)
    h1v(2.0);   // bin 1  [1, 3)
    h5h::write_hist_to_group(out_file, h1v, "h1v");
  }

  // Map-based labeled categorical: map order != axis order
  // (verifies reordering).
  {
    enum class Flav { light, c, b, tau };
    using map_meta_t = std::pair<std::string, std::map<Flav, std::string>>;
    using flav_ax_t  = bh::axis::category<Flav, map_meta_t,
                                           bh::axis::option::overflow_t>;
    auto h_fmap = bh::make_histogram(
      flav_ax_t(
        {Flav::b, Flav::light, Flav::c, Flav::tau},   // axis bin order
        map_meta_t{"flavMap", {                         // alphabetical in map
          {Flav::b, "b"}, {Flav::c, "c"},
          {Flav::light, "light"}, {Flav::tau, "tau"}
        }}
      )
    );
    h_fmap(Flav::b);      // -> bin 0
    h_fmap(Flav::light);  // -> bin 1
    h5h::write_hist_to_group(out_file, h_fmap, "h_fmap");
  }

  // External list override: std::deque, one element per axis.
  // The regular "pt" axis gets an empty vector (ignored by the
  // ContinuousAxis + LabelList fallback overload).
  {
    using cat_ax = bh::axis::category<int>;
    using dax_t  = bh::axis::regular<double>;
    auto h_olist = bh::make_histogram(
      cat_ax({0, 1, 2}, "letters"),
      cat_ax({10, 20},  "digits"),
      dax_t(2, 0.0, 2.0, "pt"));
    h_olist(0, 10, 0.5);
    std::deque<std::vector<std::string>> lbl_overrides{
      {"a", "b", "c"},
      {"ten", "twenty"},
      {}
    };
    h5h::write_hist_to_group(
      out_file, h_olist, "h_olist", lbl_overrides);
  }

  // External map override: unordered_map<string,...>.
  // "pt" is absent from the map; it uses its own metadata.
  {
    using cat_ax = bh::axis::category<int>;
    using dax_t  = bh::axis::regular<double>;
    auto h_omap = bh::make_histogram(
      cat_ax({0, 1, 2}, "letters"),
      cat_ax({10, 20},  "digits"),
      dax_t(2, 0.0, 2.0, "pt"));
    h_omap(1, 20, 1.5);
    std::unordered_map<std::string,
                       std::vector<std::string>> lbl_map{
      {"letters", {"a", "b", "c"}},
      {"digits",  {"ten", "twenty"}}
    };
    h5h::write_hist_to_group(
      out_file, h_omap, "h_omap", lbl_map);
  }

  // Error-path validation tests
  auto expect_throw = [](auto fn, const char* label) {
    bool threw = false;
    try { fn(); }
    catch (const std::invalid_argument&) { threw = true; }
    if (!threw)
      throw std::logic_error(
        std::string("expected std::invalid_argument from: ") + label);
  };

  // Test 1 — case 3: AxisOverrideSeq length != hist.rank()
  expect_throw([&]{
    using cat_ax = bh::axis::category<int>;
    auto h = bh::make_histogram(
      cat_ax({0,1}, "a"), cat_ax({0,1}, "b"));
    // 1 entry for a 2-axis histogram
    std::vector<std::vector<std::string>> short_lbl{{"x","y"}};
    h5h::write_hist_to_group(out_file, h, "err1", short_lbl);
  }, "AxisOverrideSeq size mismatch");

  // Test 2 — case 4: continuous axis key present in AxisOverrideMap
  expect_throw([&]{
    auto h = bh::make_histogram(
      bh::axis::regular<double>(2, 0.0, 2.0, "pt"),
      bh::axis::category<int>({0,1}, "flavor"));
    std::unordered_map<std::string,
                       std::vector<std::string>> m{
      {"pt", {"low","high"}}};
    h5h::write_hist_to_group(out_file, h, "err2", m);
  }, "AxisOverrideMap key for continuous axis");

  // Test 3 — case 5: non-empty labels for continuous axis in seq list
  expect_throw([&]{
    using cat_ax = bh::axis::category<int>;
    auto h = bh::make_histogram(
      cat_ax({0,1}, "flavor"),
      bh::axis::regular<double>(2, 0.0, 2.0, "pt"));
    std::vector<std::vector<std::string>> lbl{
      {"a","b"},
      {"low","high"}};
    h5h::write_hist_to_group(out_file, h, "err3", lbl);
  }, "non-empty labels for continuous axis in list");
}

int main(int, char*[]) {
  try {
    run_tests();
  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
  return 0;
}
