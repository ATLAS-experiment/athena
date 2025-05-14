/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// c++
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
      
        // Store to check later if the input ranges are within the range of the hist
        // subtract epsilon so that it doesn't go into the overflow bin at the highest edge
        m_maxX  = m_calibConstantHist->GetXaxis()->GetXmax() - std::numeric_limits<double>::epsilon();
        m_minX  = m_calibConstantHist->GetXaxis()->GetXmin() + std::numeric_limits<double>::epsilon();
        m_maxY  = m_calibConstantHist->GetYaxis()->GetXmax() - std::numeric_limits<double>::epsilon();
        m_minY  = m_calibConstantHist->GetYaxis()->GetXmin() + std::numeric_limits<double>::epsilon();
      }  
    }

    double CalibContainer::getCalibConstant(const TrackCalibObj& trk) const
    {
        // If outside the range, use the last bin in either direction
        const int binEta = m_calibConstantHist->GetXaxis()->FindFixBin(std::max(std::min(trk.eta,m_maxX),m_minX));
        const int binPhi = m_calibConstantHist->GetYaxis()->FindFixBin(std::max(std::min(trk.phi,m_maxY),m_minY));

        int gbin = m_calibConstantHist->GetBin(binEta, binPhi);

        return m_calibConstantHist->GetBinContent(gbin);
    }

}
