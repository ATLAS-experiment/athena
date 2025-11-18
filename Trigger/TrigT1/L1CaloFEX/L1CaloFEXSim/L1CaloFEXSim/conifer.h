/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CONIFER_CPP_H__
#define CONIFER_CPP_H__
#include "nlohmann/json.hpp"
#include <vector>
#include <algorithm>
#include <numeric> //std::accumulate
#include <cassert>
#include <fstream>

namespace conifer {

/* ---
 * Balanced tree reduce implementation.
 * Reduces an array of inputs to a single value using the template binary
 * operator 'Op', for example summing all elements with Op_add, or finding the
 * maximum with Op_max Use only when the input array is fully unrolled. Or,
 * slice out a fully unrolled section before applying and accumulate the result
 * over the rolled dimension. Required for emulation to guarantee equality of
 * ordering.
 * --- */
constexpr int floorlog2(int x) { return (x < 2) ? 0 : 1 + floorlog2(x / 2); }

template <int B> constexpr int pow(int x) {
  return x == 0 ? 1 : B * pow<B>(x - 1);
}

constexpr int pow2(int x) { return pow<2>(x); }

template <class T, class Op> T reduce(std::vector<T> x, Op op) {
  int N = x.size();
  int leftN = pow2(floorlog2(N - 1)) > 0 ? pow2(floorlog2(N - 1)) : 0;
  // static constexpr int rightN = N - leftN > 0 ? N - leftN : 0;
  if (N == 1) {
    return x.at(0);
  } else if (N == 2) {
    return op(x.at(0), x.at(1));
  } else {
    std::vector<T> left(x.begin(), x.begin() + leftN);
    std::vector<T> right(x.begin() + leftN, x.end());
    return op(reduce<T, Op>(std::move(left), op), reduce<T, Op>(std::move(right), op));
  }
}

template <class T> class OpAdd {
public:
  T operator()(T a, T b) { return a + b; }
};

template <class T, class U> class DecisionTree {

private:
  std::vector<int> m_feature;
  std::vector<int> m_children_left;
  std::vector<int> m_children_right;
  std::vector<T> m_threshold_;
  std::vector<U> m_value_;
  std::vector<double> m_threshold;
  std::vector<double> m_value;

public:
  U decision_function(std::vector<T> x) const {
    /* Do the prediction */
    int i = 0;
    while (m_feature[i] != -2) { // continue until reaching leaf
      bool comparison = x[m_feature[i]] <= m_threshold_[i];
      i = comparison ? m_children_left[i] : m_children_right[i];
    }
    return m_value_[i];
  }

  void init_() {
    /* Since T, U types may not be readable from the JSON, read them to double
     * and the cast them here */
    std::transform(m_threshold.begin(), m_threshold.end(),
                   std::back_inserter(m_threshold_),
                   [](double t) -> T { return (T)t; });
    std::transform(m_value.begin(), m_value.end(), std::back_inserter(m_value_),
                   [](double v) -> U { return (U)v; });
  }

  // Define how to read this class to/from JSON
  friend void from_json(const nlohmann::json &j, DecisionTree &o) {
    j.at("feature").get_to(o.m_feature);
    j.at("children_left").get_to(o.m_children_left);
    j.at("children_right").get_to(o.m_children_right);
    j.at("threshold").get_to(o.m_threshold);
    j.at("value").get_to(o.m_value);
  }

}; // class DecisionTree

template <class T, class U, bool useAddTree = false> class BDT {

private:
  int m_n_classes;
  int m_n_trees;
  int m_n_features;
  std::vector<double> m_init_predict;
  std::vector<U> m_init_predict_;
  // vector of decision trees: outer dimension tree, inner dimension class
  std::vector<std::vector<DecisionTree<T, U>>> m_trees;
  OpAdd<U> m_add;

public:
  void init(/*std::string filename*/) {
    /* Construct the BDT from conifer cpp backend JSON file */
    //  std::ifstream ifs(filename);
    //  nlohmann::json j = nlohmann::json::parse(ifs);
    //  from_json(j, *this);
    /* Do some transformation to initialise things into the proper emulation T,
     * U types */
    if (m_n_classes == 2)
      m_n_classes = 1;
    std::transform(m_init_predict.begin(), m_init_predict.end(),
                   std::back_inserter(m_init_predict_),
                   [](double ip) -> U { return (U)ip; });
    for (int i = 0; i < m_n_trees; i++) {
      for (int j = 0; j < m_n_classes; j++) {
        m_trees.at(i).at(j).init_();
      }
    }
  }

  std::vector<U> decision_function(std::vector<T> x) const {
    /* Do the prediction */
    assert("Size of feature vector mismatches expected m_n_features" &&
           x.size() == static_cast<size_t>(m_n_features));
    std::vector<U> values;
    std::vector<std::vector<U>> values_trees;
    values_trees.resize(m_n_classes);
    values.resize(m_n_classes, U(0));
    for (int i = 0; i < m_n_classes; i++) {
      std::transform(
          m_trees.begin(), m_trees.end(),
          std::back_inserter(values_trees.at(i)),
          [&i, &x](auto tree_v) { return tree_v.at(i).decision_function(x); });
      if (useAddTree) {
        values.at(i) = m_init_predict_.at(i);
        values.at(i) += reduce<U, OpAdd<U>>(values_trees.at(i), m_add);
      } else {
        values.at(i) =
            std::accumulate(values_trees.at(i).begin(),
                            values_trees.at(i).end(), U(m_init_predict_.at(i)));
      }
    }

    return values;
  }

  // Define how to read this class to/from JSON
  friend void from_json(const nlohmann::json &j, BDT &o) {
    j.at("n_classes").get_to(o.m_n_classes);
    j.at("n_trees").get_to(o.m_n_trees);
    j.at("n_features").get_to(o.m_n_features);
    j.at("init_predict").get_to(o.m_init_predict);
    j.at("trees").get_to(o.m_trees);
  }
}; // class BDT

} // namespace conifer

#endif
