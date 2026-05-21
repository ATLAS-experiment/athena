/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaBaseComps/AthHistogramming.h"

#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TestTools/initGaudi.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TEfficiency.h"
#include "TGraph.h"
#include "TTree.h"

#include <cassert>
#include <string>

class TestHistogramming : public AthHistogramming{
public:
  explicit TestHistogramming(const std::string& name)
    : AthHistogramming(name)
  {}

  StatusCode
  config(const ServiceHandle<ITHistSvc>& histSvc,
         const std::string& stream,
         const std::string& rootDir,
         const std::string& histNamePrefix = "",
         const std::string& histNamePostfix = "",
         const std::string& histTitlePrefix = "",
         const std::string& histTitlePostfix = ""){
    return configAthHistogramming(histSvc,
                                  stream,
                                  rootDir,
                                  histNamePrefix,
                                  histNamePostfix,
                                  histTitlePrefix,
                                  histTitlePostfix);
  }

  using AthHistogramming::book;
  using AthHistogramming::bookGetPointer;
  using AthHistogramming::hist;
  using AthHistogramming::hist2d;
  using AthHistogramming::hist3d;
  using AthHistogramming::tree;
  using AthHistogramming::graph;
  using AthHistogramming::efficiency;
};

void
testNullPointers(TestHistogramming& hist){
  std::cout << "testNullPointers\n";
  assert(hist.book(static_cast<TH1*>(nullptr)).isFailure());
  assert(hist.bookGetPointer(static_cast<TH1*>(nullptr)) == nullptr);
  assert(hist.book(static_cast<TEfficiency*>(nullptr)).isFailure());
  assert(hist.bookGetPointer(static_cast<TEfficiency*>(nullptr)) == nullptr);
}

void
testBookTH1(TestHistogramming& hist){
  std::cout << "testBookTH1\n";
  const TH1F h1{"h1", "h1 title", 10, 0., 10.};

  TH1* booked = hist.bookGetPointer(h1);
  assert(booked != nullptr);

  TH1* retrieved = hist.hist("h1");
  assert(retrieved != nullptr);
  assert(retrieved == booked);
  assert(std::string{retrieved->GetName()} == "h1");
  assert(std::string{retrieved->GetTitle()} == "title h1 title postfix");
}

void
testBookTH2(TestHistogramming& hist){
  TH2F h2{"h2", "h2 title", 10, 0., 10., 10, 0., 10.};

  assert(hist.book(h2).isSuccess());

  TH2* retrieved = hist.hist2d("h2");
  assert(retrieved != nullptr);
  assert(std::string{retrieved->GetName()} == "h2");
}

void
testBookTH3(TestHistogramming& hist){
  std::cout << "testBookTH3\n";
  TH3F h3{"h3", "h3 title", 10, 0., 10., 10, 0., 10., 10, 0., 10.};

  assert(hist.book(h3).isSuccess());

  TH3* retrieved = hist.hist3d("h3");
  assert(retrieved != nullptr);
  assert(std::string{retrieved->GetName()} == "h3");
}

void
testBookTree(TestHistogramming& hist){
  std::cout << "testBookTree\n";
  TTree tree{"tree", "tree title"};

  assert(hist.book(tree).isSuccess());

  TTree* retrieved = hist.tree("tree");
  assert(retrieved != nullptr);
  assert(std::string{retrieved->GetName()} == "tree");
}

void
testBookGraph(TestHistogramming& hist){
  std::cout << "testBookGraph\n";
  TGraph graph{};
  graph.SetName("graph");
  graph.SetTitle("graph title");

  TGraph* booked = hist.bookGetPointer(graph);
  assert(booked != nullptr);

  TGraph* retrieved = hist.graph("graph");
  assert(retrieved != nullptr);
  assert(retrieved == booked);
}

void
testBookEfficiency(TestHistogramming& hist){
  std::cout << "testBookEfficiency\n";

  TEfficiency eff{"eff", "eff title", 10, 0., 10.};

  TEfficiency* booked = hist.bookGetPointer(eff);
  assert(booked != nullptr);

  TEfficiency* retrieved = hist.efficiency("eff");
  assert(retrieved != nullptr);
  assert(retrieved == booked);
  assert(std::string{retrieved->GetName()} == "eff");
  assert(std::string{retrieved->GetTitle()} == "title eff title postfix");
}

int
main(){
  ISvcLocator* svcLoc = nullptr;
  assert(Athena_test::initGaudi("AthHistogramming_test.txt", svcLoc));

  ServiceHandle<ITHistSvc> histSvc{"THistSvc", "AthHistogramming_test"};
  assert(histSvc.retrieve().isSuccess());

  TestHistogramming hist{"AthHistogramming_test"};
  assert(hist.config(histSvc,
                     "AANT",
                     "/",
                     "test_",
                     "_suffix",
                     "title ",
                     " postfix").isSuccess());

  testNullPointers(hist);
  testBookTH1(hist);
  testBookTH2(hist);
  testBookTH3(hist);
  testBookTree(hist);
  testBookGraph(hist);
  testBookEfficiency(hist);

  assert(histSvc.release().isSuccess());

  return 0;
}