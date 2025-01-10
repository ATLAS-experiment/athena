/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "gtest/gtest.h"

#include "../Egamma1BDT/parameters.h"

#include "PathResolver/PathResolver.h"

#include <sstream>
#include <fstream>
#include <cmath>

TEST (egamma1BDTTester, testvecs) {
  std::string path =
    "/eos/atlas/atlascerngroupdisk/data-art/large-input/trig-val/GlobalSimTest";
 
  auto tv_fn = path + "/Egamma1BDT_testVecs.txt";
    
  // lambda to read in test vectors

  auto get_test_vectors =
    [&tv_fn]()->std::optional<std::vector<std::vector<double>>> {
    std::ifstream ifs(tv_fn);


    if(!ifs) {
      return std::optional<std::vector<std::vector<double>>>();
    }
      
    auto line = std::string();
    std::string tok;

    std::vector<std::vector<double>> inputs;

    while (std::getline(ifs, line)) {
      auto ss = std::stringstream(line);
      std::vector<double> input;
	
      while (std::getline(ss, tok, ',')) {
	input.push_back(std::stod(tok));
      }

      constexpr int exp_size{18};
      if (input.size() != exp_size) {
	return std::optional<std::vector<std::vector<double>>>();
      }

      inputs.push_back(input);
    }
      
    return std::optional(inputs);
      
  };

  auto test_vectors_opt = get_test_vectors();

  EXPECT_EQ(test_vectors_opt.has_value(), true);

    
  auto test_vectors = *test_vectors_opt;

  std::string fn_exp = path + "/Egamma1BDT_testExps_hls.txt";
     
  // lambda to get test expectations
  auto get_expectations =
    [&fn_exp]() -> std::optional<std::vector<double>> {

    std::ifstream ifs(fn_exp);
    if(!ifs) {
      return std::optional<std::vector<double>>();
    }
      
    auto line = std::string();
    std::string tok;
    std::vector<double> exps;
      
    while (std::getline(ifs, line)) {
      auto ss = std::stringstream(line);
      while (std::getline(ss, tok)) {
	exps.push_back(std::stod(tok));
      }
    }
      
    return exps;
      
  };


  auto expectations_opt = get_expectations();

  EXPECT_EQ(expectations_opt.has_value(), true);

  auto expectations = *expectations_opt;
    
  EXPECT_EQ(test_vectors.size(), expectations.size());

  const auto tv_sz = test_vectors.size();
  GlobalSim::input_arr_t x;
  std::size_t i_vec{0};
  for (std::size_t i_test = 0; i_test != tv_sz; ++i_test) {
    const auto& tv = test_vectors[i_test];
    const auto& exp = expectations[i_test];
      
    auto tv_size = tv.size();
    for(std::size_t j = 0; j != tv_size; ++j) {
      x[j] = tv[j];
    }

    GlobalSim::score_arr_t score{};

    //main function for egamma BDT. object instantiated at start up
    GlobalSim::bdt.decision_function(x, score);
      
    EXPECT_EQ(score[0], exp);
    ++i_vec;
  }

  EXPECT_EQ(i_vec, 32602u);
}
