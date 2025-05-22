/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "dqm_algorithms/TileBinsOutRange.h"

#include "TProfile2D.h"
#include <array>
#include <iostream>


void printHistogram(const TH1* histogam, const std::string & name) {
  std::cout << "===========>  BEGIN Histogram: " << name << "  <============" << std::endl;
  histogam->Print("All");
  std::cout << "------------>  END Histogram: " << name << "  <--------------" << std::endl;
}

void TestTileBinsOutRange(void) {

  auto algorithm = new dqm_algorithms::TileBinsOutRange();
  algorithm->printDescription(std::cout);

  std::unique_ptr<TProfile2D> histogram(new TProfile2D("histogram", "Simple Profile", 2, 0, 2, 2, 0, 2));
  histogram->SetDirectory(0);
  for (int i = 0; i < 10; ++i) {
    // Fill good values
    histogram->Fill(0.5, 0.5, 5);
    histogram->Fill(1.5, 0.5, 15);
  }

  std::unique_ptr<dqm_core::test::DummyAlgorithmConfig> config(new dqm_core::test::DummyAlgorithmConfig());
  config->addParameter("MinBinEntries", 2);

  // Threshold for number of bins aways from threshold
  config->addGreenThreshold("NBins", 0);
  config->addGreenThreshold("MinValue", -50);
  config->addGreenThreshold("MaxValue", 50);

  config->addRedThreshold("NBins", 2);

  std::cout << "Config: ";
  config->print(std::cout);

  bool isTestOk = true;

  std::cout << std::endl << std::endl;
  // Fill bad values, but number of bin entries below MinBinEntries (2)
  histogram->Fill(0.5, 1.5, -100);
  histogram->Fill(1.5, 1.5, 100);

  printHistogram(histogram.get(), "Green");
  std::unique_ptr<dqm_core::Result> greenResult(algorithm->execute( "test", *histogram, *config));
  std::cout << "Result: " << *(greenResult.get()) << std::endl;
  isTestOk &= (greenResult->status_ == dqm_core::Result::Green);

  std::cout << std::endl;
  histogram->Fill(0.5, 1.5, -100);
  printHistogram(histogram.get(), "Yellow");
  std::unique_ptr<dqm_core::Result> yellowResult(algorithm->execute( "test", *histogram, *config));
  std::cout << "Result: " << *(yellowResult.get()) << std::endl;
  isTestOk &= (yellowResult->status_ == dqm_core::Result::Yellow);

  std::cout << std::endl;
  histogram->Fill(1.5, 1.5, 100);
  printHistogram(histogram.get(), "Red");
  std::unique_ptr<dqm_core::Result> redResult(algorithm->execute( "test", *histogram, *config));
  std::cout << "Result: " << *(redResult.get()) << std::endl;
  isTestOk &= (redResult->status_ == dqm_core::Result::Red);

  std::cout << std::endl;
  std::cout << "===================== >>>  SUMMARY <<< =====================" << std::endl;
  std::cout << "ALL TESTS:\t" << (isTestOk ? "PASSED" : "FAILED") << std::endl;
}
