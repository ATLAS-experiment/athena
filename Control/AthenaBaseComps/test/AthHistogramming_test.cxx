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
#include <iostream>
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

  assert(std::string{retrieved->GetName()} == "test_h1_suffix");
  assert(std::string{retrieved->GetTitle()} == "title h1 title postfix");
}

void
testBookTH2(TestHistogramming& hist){
  std::cout << "testBookTH2\n";

  const TH2F h2{"h2", "h2 title", 10, 0., 10., 10, 0., 10.};

  assert(hist.book(h2).isSuccess());

  TH2* retrieved = hist.hist2d("h2");
  assert(retrieved != nullptr);
  assert(std::string{retrieved->GetName()} == "test_h2_suffix");
}

void
testBookTH3(TestHistogramming& hist){
  std::cout << "testBookTH3\n";

  const TH3F h3{"h3", "h3 title", 10, 0., 10., 10, 0., 10., 10, 0., 10.};

  assert(hist.book(h3).isSuccess());

  TH3* retrieved = hist.hist3d("h3");
  assert(retrieved != nullptr);
  assert(std::string{retrieved->GetName()} == "test_h3_suffix");
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

  /*
    TGraph currently differs from TH1/TEfficiency: the cloned graph object
    receives the configured name prefix/postfix.
  */
  assert(std::string{retrieved->GetName()} == "test_graph_suffix");
  assert(std::string{retrieved->GetTitle()} == "title graph title postfix");
}

void
testBookConstEfficiency(TestHistogramming& hist){
  std::cout << "testBookConstEfficiency\n";

  const TEfficiency eff{"constEff", "const eff title", 10, 0., 10.};

  TEfficiency* booked = hist.bookGetPointer(eff);
  assert(booked != nullptr);

  TEfficiency* retrieved = hist.efficiency("constEff");
  assert(retrieved != nullptr);
  assert(retrieved == booked);

  assert(std::string{retrieved->GetName()} == "test_constEff_suffix");
  assert(std::string{retrieved->GetTitle()} == "title const eff title postfix");
}




void
testTH1CanBeRetrievedByBareNameAfterBookingWithDirectoryName(TestHistogramming& hist){
  std::cout << "testTH1CanBeRetrievedByBareNameAfterBookingWithDirectoryName\n";

  const TH1F h{"dir/bareLookup", "bare lookup title", 10, 0., 10.};

  TH1* booked = hist.bookGetPointer(h);
  assert(booked != nullptr);

  /*
    buildBookingString strips "dir/" from the object name and appends it to
    the booking directory. The public lookup by the stripped name should
    retrieve the booked histogram.
  */
  TH1* retrieved = hist.hist("bareLookup");
  assert(retrieved != nullptr);
  assert(retrieved == booked);
  assert(std::string{retrieved->GetName()} == "test_bareLookup_suffix");
}

void
testTH1CanBeRetrievedByQualifiedNameAfterBookingWithDirectoryName(TestHistogramming& hist){
  std::cout << "testTH1CanBeRetrievedByQualifiedNameAfterBookingWithDirectoryName\n";

  const TH1F h{"dir/qualifiedLookup", "qualified lookup title", 10, 0., 10.};

  TH1* booked = hist.bookGetPointer(h);
  assert(booked != nullptr);

  /*
    This documents the public behaviour that a directory-qualified lookup is
    also accepted.

    With the current implementation, this may be rescued by a THistSvc lookup
    rather than by the local cache, because hist("dir/qualifiedLookup") hashes
    the unnormalised name before buildBookingString strips "dir/".
  */
  TH1* retrieved = hist.hist("dir/qualifiedLookup");
  assert(retrieved != nullptr);
  assert(std::string{retrieved->GetName()} == "test_qualifiedLookup_suffix");
  assert(retrieved == booked);
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
  testBookConstEfficiency(hist);

  testTH1CanBeRetrievedByBareNameAfterBookingWithDirectoryName(hist);
  testTH1CanBeRetrievedByQualifiedNameAfterBookingWithDirectoryName(hist);

  assert(histSvc.release().isSuccess());

  return 0;
}