/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// c++
#include <algorithm>
#include <iostream>
#include <stdexcept>

// Local include(s):
#include <MuonMomentumCorrections/CalibContainer.h>
#include "PathResolver/PathResolver.h"

// Root
#include <TFile.h>

namespace MCP {

    CalibContainer::CalibContainer(const std::string& inFileName, const std::string& histName)
    {

      if (inFileName.empty()) throw std::invalid_argument("fileName arguments must be non empty");
      if (histName.empty()) throw std::invalid_argument("histName arguments must be non empty");
      
      auto fileName = PathResolverFindCalibFile(inFileName);
      
      std::unique_ptr<TFile> fmc{TFile::Open(fileName.c_str(), "READ")};
      if (!fmc || !fmc->IsOpen()) 
        {
	  throw std::invalid_argument("Cannot open file " + fileName);
        }
      
      TH2* hist = nullptr;
      fmc->GetObject(histName.c_str(), hist);
      if (!hist) {
          if (histName.find("ptExtra") != std::string::npos) {
            // exceptional condition, to make it compatible with old recommendations
            m_calibConstantHist.reset(nullptr);
          } else {
	    throw std::invalid_argument("Cannot find hist ("+histName+") in file " + fileName);
          }
      } else {
        hist->SetDirectory(nullptr);
        m_calibConstantHist.reset(hist);
      }  
    }

    double CalibContainer::getCalibConstant(const TrackCalibObj& trk) const
    {
        // If outside the range, use the last bin in either direction
        const int binEta = std::clamp(m_calibConstantHist->GetXaxis()->FindFixBin(trk.eta), 1, m_calibConstantHist->GetNbinsX());
        const int binPhi = std::clamp(m_calibConstantHist->GetYaxis()->FindFixBin(trk.phi), 1, m_calibConstantHist->GetNbinsY());

        int gbin = m_calibConstantHist->GetBin(binEta, binPhi);

        return m_calibConstantHist->GetBinContent(gbin);
    }

}
