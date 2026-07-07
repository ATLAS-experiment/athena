/*
  Copyright (C) 2002-2026 for the benefit of the ATLAS collaboration
*/

// Demonstration: Workbench script for testing output of new dqm algorithm
// Author: Cary David Randazzo, November 2025, Louisiana Tech University

#include "PathResolver/PathResolver.h"

void TestAutoencoderExampleAlgorithm()
{

    dqm_algorithms::AutoencoderExampleAlgorithm *algorithm = new dqm_algorithms::AutoencoderExampleAlgorithm();

    // Here, we will test on a histogram from within a Root file.

    // Load the root file from its native location. Make sure you have permissions to access that location.
    // NOTE: eos does not have guaranteed availability so PathResolver is used to find asg-calibdev area for non-production development.
    const std::string PathResolverFindCalibFile("dev/ONNXfiles/data25_13p6TeV.00502502.physics_Main.merge.HIST.f1608_h531._0001.1")

    TFile *file = TFile::Open(test_file_path);
    if (!file || file->IsZombie()) {
      std::cerr << "Error: Cannot open ROOT file!" << std::endl;
      file->Close();
      return;
    }

    // Load the histogram from the Root file
    TH2F *histogram = nullptr;

    // Provide the path to the histogram you want to test.
    // NOTE: You may choose to test your own root file, directory, histogram according to the path above.
    // Example:
    const char *hpath = "run_502502/Jets/AntiKt4LCTopoJets/highptrange200GeVto500GeV/eta_phi_highptrange200GeVto500GeV";

    file->GetObject(hpath, histogram);
    if (!histogram) {
      std::cerr << "ERROR: histogram not found: " << hpath << "\n";
      file->Close();
      return;
    }

    histogram->SetDirectory(nullptr);
    gROOT->cd();
    file->Close();


    // Here, the workbench tests the algorithm and outputs can be viewed as expected in your IDE.
    dqm_core::test::DummyAlgorithmConfig *aconfig = new dqm_core::test::DummyAlgorithmConfig(histogram);
    dqm_core::Result *result = algorithm->execute("test", *histogram, *aconfig );
    algorithm->printDescription();
    std::cout << "Result " << result->status_<< std::endl;
}
