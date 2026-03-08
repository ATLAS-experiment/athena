/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

// **********************************************************************
// L1Calo post processing jobs
// Author: Peter Faulkner
// **********************************************************************

#include "DataQualityUtils/MonitoringFile.h"

#include <string>
#include <format>

#include <TDirectory.h>
#include <TFile.h>
#include <TH1.h>
#include <TKey.h>
#include <TObject.h>

#ifndef L1CALOPOSTPROCESSDEBUG
#define L1CALOPOSTPROCESSDEBUG false
#endif

namespace dqutils {
  //--------------------------------------------------------------------------------
  // main process
  //--------------------------------------------------------------------------------
  void MonitoringFile::L1CaloPostProcess(const std::string& inFilename, bool /* isIncremental */) {
    //bool debug = L1CALOPOSTPROCESSDEBUG;

    //if (debug) std::cout << "--> L1CaloPostProcess: Begin L1Calo post-processing" << std::endl;

    //open root file
    TFile* f = TFile::Open(inFilename.c_str(), "UPDATE");

    //check files are loaded.
    if (f == 0 || !f->IsOpen()) {
      std::cerr << "--> L1CaloPostProcess: Input file not opened" << std::endl;
      delete f;
      return;
    }

    //zombie files?
    if (f->IsZombie()) {
      std::cerr << "--> L1CaloPostProcess: Input file " << inFilename << " cannot be opened. " << std::endl;
      delete f;
      return;
    }

    //check file size is not too small.
    if (f->GetSize() < 1000.) {
      std::cerr << "--> L1CaloPostProcess: Input file empty" << std::endl;
      f->Close();
      delete f;
      return;
    }

    //build iterator
    TIter next_run(f->GetListOfKeys());
    TKey* key_run(0);

    //loop over keys in root directory
    while ((key_run = dynamic_cast<TKey*>(next_run())) != 0) {
      TObject* obj_run = key_run->ReadObj();
      TDirectory* tdir_run = dynamic_cast<TDirectory*>(obj_run);

      //check we have a valid pointer
      if (tdir_run == 0) {
        delete obj_run;
        continue;
      }

      //check name
      std::string runDirName(tdir_run->GetName());
      //if (debug) std::cout << "Run_directory: " << runDirName << std::endl;

      if (runDirName.find("run") == std::string::npos) {
        delete obj_run;
        continue;
      }

      // Stability RMS histograms
      std::string stabilityDirName = runDirName + "/L1Calo/PPM/Stability";
      L1CaloStabilityRMS(f, stabilityDirName + "/FineTime", "fineTime");
      L1CaloStabilityRMS(f, stabilityDirName + "/Pedestal", "pedestal");
      L1CaloStabilityRMS(f, stabilityDirName + "/EtCorrelation", "etCorrelation");

      // Trigger efficiency eta-phi histograms
      int items = 16;
      int binSkip = 8;
      double threshold[6] = {
        3., 3., 3., 6., 4., 2.
      };
      static const std::string energies[6] = {
        "10", "20", "30", "50", "100", "200"
      };
      for (int i = 0; i < 3; ++i) {
        std::string effDir = runDirName + "/L1Calo/Reco/EmEfficiencies/ClusterRaw_"
                             + energies[i] + "GeV_EtaVsPhi";
        std::string nameDen = "ClusterRaw_" + energies[i] + "GeV_Eta_vs_Phi";
        std::string nameEff = nameDen + "_trig_Eff";
        L1CaloResetEfficiencies(f, effDir, nameDen, nameEff, items, threshold[i], binSkip);
      }
      items = 8;
      binSkip = 6;
      int itemsF = 4;
      int binSkipF = 0;
      for (int i = 3; i < 6; ++i) {
        std::string effDir = runDirName + "/L1Calo/Reco/JetEfficiencies/JetEmScale_"
                         + energies[i] + "GeV_EtaVsPhi";
        std::string nameDen = "JetEmScale_" + energies[i] + "GeV_Eta_vs_Phi";
        std::string nameEff = nameDen + "_J_Eff_item";
        std::string nameEffF = nameDen + "_FJ_J_Eff_item";
        L1CaloResetEfficiencies(f, effDir, nameDen, nameEff, items, threshold[i], binSkip);
        L1CaloResetEfficiencies(f, effDir, nameDen, nameEffF, itemsF, threshold[i], binSkipF);
      }
    }

    f->Close();
    delete f;
    //if (debug) std::cout << "--> L1CaloPostProcess: finished L1Calo post-processing"
    //                     << std::endl;
  }

  // Get RMS from stability profiles

  void MonitoringFile::L1CaloStabilityRMS(TFile* f, const std::string& nameDir,
                                          const std::string& nameTag) {
    std::string nameData("ppm_em_2d_profile_etaPhi_adc_" + nameTag);
    std::string nameError("ppm_em_2d_etaPhi_adc_" + nameTag + "RMS");

    L1CaloFillWithError(f, nameDir, nameData, nameError);
    nameData = "ppm_had_2d_profile_etaPhi_adc_" + nameTag;
    nameError = "ppm_had_2d_etaPhi_adc_" + nameTag + "RMS";
    L1CaloFillWithError(f, nameDir, nameData, nameError);
  }

  // Fill second histogram with error from first

  void MonitoringFile::L1CaloFillWithError(TFile* f, const std::string& nameDir,
                                           const std::string& nameData, const std::string& nameError) {
    //const bool debug = L1CALOPOSTPROCESSDEBUG;

    // Check directory
    if (!(f->GetDirectory(nameDir.c_str()))) {
      //if (debug) std::cout << "--> L1CaloPostProcess: directory "
      //                     << nameDir << " not found." << std::endl;
      return;
    }
    if (f->cd(nameDir.c_str()) == 0) {
      //if (debug) std::cout << "dir " << nameDir << " isn't there!" << std::endl;
      return;
    }

    // Data histogram
    std::string p1 = nameDir + "/" + nameData;
    if (!CheckHistogram(f, p1.c_str())) {
      //if (debug) std::cout << " histo " << p1.Data() << " is not in file "
      //                     << f->GetName() << std::endl;
      return;
    }
    TH1* h1 = (TH1*) (f->Get(p1.c_str()));

    // Error histogram
    std::string p2 = nameDir + "/" + nameError;
    if (!CheckHistogram(f, p2.c_str())) {
      //if (debug) std::cout << " histo " << p2.Data() << " is not in file "
      //                     << f->GetName() << std::endl;
      return;
    }
    TH1* h2 = (TH1*) (f->Get(p2.c_str()));

    // Consistency checks
    const int dim = h1->GetDimension();
    if (dim != h2->GetDimension()) {
      //if (debug) std::cout << "Histograms have different dimension" << std::endl;
      return;
    }
    if (h1->GetNbinsX() != h2->GetNbinsX() ||
        ((dim == 2) && (h1->GetNbinsY() != h2->GetNbinsY()))) {
      //if (debug) std::cout << "Histograms have different number of bins" << std::endl;
      return;
    }

    // Fill error histogram
    h2->Reset();
    int nbins = h1->GetNbinsX() + 2;
    if (dim == 2) nbins *= (h1->GetNbinsY() + 2);
    for (int bin = 0; bin < nbins; ++bin) {
      double error = h1->GetBinError(bin);
      if (error != 0.) h2->SetBinContent(bin, error);
    }

    // And save
    h2->Write("", TObject::kOverwrite);
    f->Write();
  }

  // Reset efficiencies to 100% for bins with low stats

  void MonitoringFile::L1CaloResetEfficiencies(TFile* f, const std::string& effDir,
                                               const std::string& nameDen, const std::string& nameEff,
                                               int items, double threshold, int binSkip) {
    //const bool debug = L1CALOPOSTPROCESSDEBUG;

    // Check directory
    if (!(f->GetDirectory(effDir.c_str()))) {
      //if (debug) std::cout << "--> L1CaloPostProcess: directory "
      //                     << effDir << " not found." << std::endl;
      return;
    }
    if (f->cd(effDir.c_str()) == 0) {
      //if (debug) std::cout << "dir " << effDir << " isn't there!" << std::endl;
      return;
    }

    // Denominator histogram
    std::string denPath = effDir + "/denominator/" + nameDen;
    TH1* h1 = (TH1*) (f->Get(denPath.c_str()));
    if (!h1) {
      //if (debug) std::cout << " histo " << denPath << " is not in file "
      //                     << f->GetName() << std::endl;
      return;
    }
    if (h1->GetDimension() != 2) {
      //if (debug) std::cout << " histo " << denPath << " has unexpected dimension"
      //                     << std::endl;
      return;
    }

    // Efficiency histograms
    int xbins = h1->GetNbinsX();
    int ybins = h1->GetNbinsY();
    std::vector<TH1*> effVec;
    std::string effBase = effDir + "/" + nameEff;
    for (int i = 0; i < items; ++i) {
      std::string effPath = effBase + std::format("_{:d}", i);
      TH1* h2 = (TH1*) (f->Get(effPath.c_str()));
      if (!h2) {
        //if (debug) std::cout << " histo " << effPath << " is not in file "
        //                     << f->GetName() << std::endl;
        continue;
      }
      if (h2->GetDimension() != 2) {
        //if (debug) std::cout << " histo " << effPath << " has unexpected dimension"
        //                     << std::endl;
        continue;
      }
      if (xbins != h2->GetNbinsX() || ybins != h2->GetNbinsY()) {
        //if (debug) std::cout << " histos " << denPath << " and " << effPath
        //                     << " have different number of bins" << std::endl;
        continue;
      }
      effVec.push_back(h2);
    }
    if (effVec.empty()) {
      //if (debug) std::cout << "No valid histograms in " << effDir << std::endl;
      return;
    }
    std::vector<TH1*>::iterator iter;
    std::vector<TH1*>::iterator iterEnd = effVec.end();

    // Loop over bins in denominator looking for entries below threshold
    for (int x = 1 + binSkip; x <= xbins - binSkip; ++x) {
      for (int y = 1; y <= ybins; ++y) {
        if (h1->GetBinContent(x, y) < threshold) {
          // Loop over efficiency histograms and set bin to 100% efficiency
          for (iter = effVec.begin(); iter != iterEnd; ++iter) {
            (*iter)->SetBinContent(x, y, 100.0);
          }
        }
      }
    }

    // Save the updated histograms
    for (iter = effVec.begin(); iter != iterEnd; ++iter) {
      (*iter)->Write("", TObject::kOverwrite);
    }
    f->Write();
  }
}

#undef L1CALOPOSTPROCESSDEBUG
