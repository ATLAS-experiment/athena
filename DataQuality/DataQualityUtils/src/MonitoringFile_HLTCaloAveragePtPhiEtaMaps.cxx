/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/* HLTCalo Post Processing Method: to produce eta phi map of the average hit energy in each bin, made
 *                                 from two 2-d histograms, one with total energy in each bin the
 *                                 other with number of hits per bin
 * Author   : Gareth Brown <gabrown@CERN.CH> based on HLTMET Post Processing
 * Date     : Oct 2011
 */

#include "DataQualityUtils/MonitoringFile.h"

#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>

#include "TH2F.h"
#include "TFile.h"
#include "TKey.h"

namespace dqutils {
  void MonitoringFile::HLTCaloAveragePtPhiEtaMaps(TFile* f, std::string& run_dir) {
    int debugLevel = MonitoringFile::getDebugLevel();

    if (debugLevel > 1) std::cout << "--> HLTCaloAveragePtPhiEtaMaps: <Quantity(eta,phi)>  = Quantity(eta,phi)/N(eta,phi) " << std::endl;

    f->cd("/");
    TIter next_run(f->GetListOfKeys());
    TKey* key_run(0);
    while ((key_run = dynamic_cast<TKey*> (next_run())) != 0) {
      if (!key_run->IsFolder()) continue;
      run_dir = key_run->GetName();
      if (run_dir.find("run") == std::string::npos) {
        continue;
      }

      std::string run_dir2 = run_dir;

      // all merged root files have the structure "rootfile.root:/run_NNNNNNN"
      // use that to extract run number
      //int run_number = atoi( (run_dir2.substr(4, run_dir2.size()-4 )).c_str() );
      //run_number=run_number;

      // begin HLTMET
      // note 1: prefix all dirs and hists with '/'
      // note 2: missing dir => return
      // note 3: missing hist => continue
      std::string hlt_top = run_dir + "/HLT"; // toplevel

      std::vector<std::string> calo_fold;
      std::vector<std::pair<std::string, std::string> > hist_numr;

      // The folders that we want to get the hists from
      calo_fold.push_back("/CaloMon");
      calo_fold.push_back("/CaloMonL2");

      // check if the folders are in hlt
      for (std::vector<std::string>::iterator it = calo_fold.begin(); it != calo_fold.end(); ++it) {
        std::string theHistDir = hlt_top + *it;
        TDirectory* dir = f->GetDirectory(theHistDir.c_str());
        if (!dir) {
          if (debugLevel > 0) std::cerr << "--> HLTCaloAveragePtPhiEtaMaps: directory " << theHistDir << " not found" << std::endl;
          return;
        }
      }

      // pairs of Num and Dem
      hist_numr.push_back(std::make_pair<std::string, std::string>("/EnergyAccetaphiLAr", "/HitAccetaphiLAr"));
      hist_numr.push_back(std::make_pair<std::string, std::string>("/EnergyAccetaphiTile", "/HitAccetaphiTile"));


      // we have all dirs, get the component histograms
      for (std::vector<std::string>::iterator itFex = calo_fold.begin(); itFex != calo_fold.end(); ++itFex) {
        for (std::vector<std::pair<std::string, std::string> >::iterator itNum = hist_numr.begin(); itNum != hist_numr.end(); ++itNum) {
          TH2F* hnum(0), *hden(0);

          // prepend histogram name with path and append with suffix [_00 .., _24 for each component]
          std::string thePath = hlt_top + (*itFex);
          std::string numPath = thePath + (*itNum).first;
          std::string denPath = thePath + (*itNum).second;

          // get histograms
          hnum = (TH2F*) (f->Get(numPath.c_str()));
          hden = (TH2F*) (f->Get(denPath.c_str()));
          // test if histograms are present
          if (!hnum) {
            if (debugLevel > 0) std::cerr << "--> HLTCaloAveragePtPhiEtaMaps: no histogram " << numPath << std::endl;
            continue;
          }
          if (!hden) {
            if (debugLevel > 0) std::cerr << "--> HLTCaloAveragePtPhiEtaMaps: no histogram " << denPath << std::endl;
            continue;
          }


          // get directory of histograms
          TDirectory* dir = f->GetDirectory(thePath.c_str());

          // these are disabled, because we have to worry about updating metadata
          // clone numerator histogram in the same directory; prepend with "avg_"
          // use numerator to do the job

          // divide num by den to get average quantity.
          hnum->Divide(hnum, hden);

          // fix histogram titles
          std::string title = hnum->GetTitle();
          hnum->SetTitle("Average Transverse Energy per eta/phi bin");

          dir->cd();
          hnum->Write("", TObject::kOverwrite);
        } // done looping over quantities
      } // done looping over directories
    } // end while loop over all keys
  } // end method MonitoringFile::HLTCaloAveragePtPhiEtaMaps
} // end namespace
