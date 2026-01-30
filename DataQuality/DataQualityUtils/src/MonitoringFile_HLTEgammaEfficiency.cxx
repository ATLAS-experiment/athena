/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

// **********************************************************************
// HLT Egamma post processing jobs
// 15/03/2012 initial file by Yan-Jie Schnellbach, based on HLT Tau
// **********************************************************************

#include "DataQualityUtils/MonitoringFile.h"

#include <cmath>
#include <vector>

#include <TCanvas.h>
#include <TF1.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TKey.h>
#include <TMath.h>
#include <TProfile.h>

namespace dqutils {
  //-------------------------------------------------------------
  // function to calculate bootstrap efficiency w.r.t to offline
  // ------------------------------------------------------------
  void MonitoringFile::HLTEgammaEfficiencyOff(TFile* f, TDirectory* trig_dir,
                                              TDirectory* off_dir, const std::string& pathNum,
                                              const std::string& pathEff, const std::vector<std::string>& varName) {
    //get the relevant paths
    std::string chain_path = getPath(trig_dir);
    std::string offline_path = getPath(off_dir);

    //establish base path (to save typing "/PassedChain/" over and over)
    std::string basePath = chain_path + "/PassedChain/";
    if (f->cd(basePath.c_str()) == 0) {
      std::cerr << "----> HLTEgammaPostProcess: basePath not found in "
                << chain_path << ", skipping now!\n";
      return;
    }

    //create histogram matrix with pointers to our histograms in question
    // 3x3 matrix, rows are variables (Et, Eta, Phi), colums the different
    // histograms (numerator - matched offline, denominator - offline, eff)
    TH1F* histo_matrix[3][3];

    //check variables to avoid segfaults!
    if (varName.size() > 3) {
      std::cerr << "----> HLTEgammaPostProcess: Too many variables "
                << "used in " << chain_path + "/PassedChain/" + pathEff
                << " for bootstrap efficiencies, skipping now!\n";
      return;
    }



    for (uint iVar = 0; iVar < varName.size(); ++iVar) {
      //form names for all variables
      std::string numName = basePath + pathNum + "eg" + varName[iVar];
      std::string denName = offline_path + "/" + "eg" + varName[iVar];
      std::string effName = basePath + pathEff + "eg" + varName[iVar];

      //check histogram existence
      if (!CheckHistogram(f, numName.c_str())) {
        std::cerr << "----> HLTEgammaPostProcess: Histogram "
                  << numName << " does not exist in file "
                  << f->GetName() << " - skipping!\n";
        return;
      }
      if (!CheckHistogram(f, denName.c_str())) {
        std::cerr << "----> HLTEgammaPostProcess: Histogram "
                  << denName << " does not exist in file "
                  << f->GetName() << " - skipping!\n";
        return;
      }
      if (!CheckHistogram(f, effName.c_str())) {
        std::cerr << "----> HLTEgammaPostProcess: Histogram "
                  << effName << " does not exist in file "
                  << f->GetName() << " - skipping!\n";
        return;
      }

      //debug outs
      //std::cout << "---->HLTEgammaPostProcess: Taking " << numName << " and divide by "
      //		<< denName << " to get " << effName << std::endl;

      //now get histograms
      histo_matrix[iVar][0] = (TH1F*) (f->Get(numName.c_str()));
      histo_matrix[iVar][1] = (TH1F*) (f->Get(denName.c_str()));
      histo_matrix[iVar][2] = (TH1F*) (f->Get(effName.c_str()));

      //hop into efficiency path
      std::string writePath = basePath + pathEff;
      f->cd(writePath.c_str());

      //create efficiency histogram by division
      histo_matrix[iVar][2]->Divide(histo_matrix[iVar][0], histo_matrix[iVar][1],
                                    1.0, 1.0, "B");
      histo_matrix[iVar][2]->Scale(100.);
      if (histo_matrix[iVar][2]->Write("",
                                       TObject::kOverwrite) ==
          0) std::cerr << "----> HLTEgammaPostProcess: Efficiency histogram "
                       << effName <<
          " not written! Histogram will be buggy!\n";
    }

    //save results now
    f->Write();
  }

  // --------------------------------------------------
  // function called to calculate relative efficiencies
  // ---------------------------------------------------

  void MonitoringFile::HLTEgammaEfficiencyRel(TFile* f, TDirectory* trig_dir,
                                              const std::string& pathPre, const std::string& pathRej,
                                              const std::vector<std::string>& objStage,
                                              const std::vector<std::string>& varName) {
    std::string chain_path = getPath(trig_dir);

    //     std::cout << "----> HLTEgammaPostProcess in " << chain_path
    //            << ", pathPre=" << chain_path + "/PassedChain/" + pathPre
    //            << ", pathRej=" << chain_path + "/PassedChain/" + pathRej

    //establish base path (to save typing)
    std::string basePath = chain_path + "/PassedChain/";
    if (f->cd(basePath.c_str()) == 0) {
      std::cerr << "----> HLTEgammaPostProcess: basePath not found in "
                << chain_path << ", skipping now!\n";
      return;
    }

    //create histogram matrix with pointers to our histograms in question
    // 5x3x3 tensor, 1st index is the stage 2nd index are variables
    // (Et, Eta, Phi), 3rd index the different histograms
    // (numerator - n-1 stage, denominator - n stage)
    TH1F* histo_tensor[5][3][3];

    //now check size of our stages and variables to avoid segfaults!
    if (objStage.size() > 5) {
      std::cerr << "----> HLTEgammaPostProcess: Too many chain stages "
                << "found in " << chain_path << " for relative efficiencies, bailing it!\n";
      return;
    }
    if (varName.size() > 3) {
      std::cerr << "----> HLTEgammaPostProcess: Too many variables "
                << "used in " << chain_path << " for relative efficiencies, bailing out!\n";
      return;
    }

    //iterate through variables (start at 1, as there is no we can't
    //calculate rejection efficiency w.r.t n-1 with n = 0 -> segfault!)
    for (uint iVar = 0; iVar < varName.size(); ++iVar) {
      //iterate through stages
      for (uint iStg = 1; iStg < objStage.size(); ++iStg) {
        //form names for all variables
        std::string preName = basePath + pathPre + objStage[iStg - 1] + varName[iVar];
        std::string rejName = basePath + pathPre + objStage[iStg] + varName[iVar];
        std::string effName = basePath + pathRej + objStage[iStg] + varName[iVar];
        //check histogram existence
        if (!CheckHistogram(f, rejName.c_str())) {
          std::cerr << "----> HLTEgammaPostProcess: Histogram "
                    << rejName << " does not exist in file "
                    << f->GetName() << " - skipping it!\n";
          return;
        }
        if (!CheckHistogram(f, preName.c_str())) {
          std::cerr << "----> HLTEgammaPostProcess: Histogram "
                    << preName << " does not exist in file "
                    << f->GetName() << " - skipping it!\n";
          return;
        }
        if (!CheckHistogram(f, effName.c_str())) {
          std::cerr << "----> HLTEgammaPostProcess: Histogram "
                    << effName << " does not exist in file "
                    << f->GetName() << " - skipping it!\n";
          return;
        }

        //debug outs
        //std::cout << "---->HLTEgammaPostProcess: Taking " << rejName << " and divide by "
        //	            << preName << " to get " << effName << std::endl;

        //now get our histograms
        histo_tensor[iStg][iVar][0] = (TH1F*) (f->Get(rejName.c_str()));
        histo_tensor[iStg][iVar][1] = (TH1F*) (f->Get(preName.c_str()));
        histo_tensor[iStg][iVar][2] = (TH1F*) (f->Get(effName.c_str()));

        //hop into rejection path
        std::string writePath = basePath + pathRej;
        f->cd(writePath.c_str());

        //create rejection efficiency histograms by division
        histo_tensor[iStg][iVar][2]->Divide(histo_tensor[iStg][iVar][0], histo_tensor[iStg][iVar][1],
                                            1.0, 1.0, "B");
        histo_tensor[iStg][iVar][2]->Scale(100.);
        if (histo_tensor[iStg][iVar][2]->Write("", TObject::kOverwrite) == 0) std::cerr
            << "---->HLTEgammaPostProcess: Relative efficiency histogram "
            << effName << " not written! Histograms will be buggy!\n";
      }
    }

    //save results now
    f->Write();
  }
}
