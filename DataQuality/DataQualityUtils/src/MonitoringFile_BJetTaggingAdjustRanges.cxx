/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/* Methods to perform post-processing on run_nnnnnn/HLT/JetMon* histograms
 * Mainly to adjust the impact parameter ranges according to the recorded lumi blocks.
 *
 *
 *
 * Based on the HLTJetCalcEfficiencyAndRate code of Venkatesh Kaushik (venkat.kaushik@cern.ch).
 *
 * Author   : Manuel Neumann (mneumann@cern.ch)
 * Date     : Aug 2011
 */

#include "DataQualityUtils/MonitoringFile.h"



#include "TH1F.h"
#include "TFile.h"
#include "TKey.h"
#include <vector>
#include <algorithm> //std::max

namespace dqutils
{
  void
  MonitoringFile::BJetTaggingAdjustRanges(TFile* f) {
    int debugLevel = MonitoringFile::getDebugLevel();

    if (debugLevel > 1) {
      std::cout << "--> BJetTaggingAdjustRanges: Adjusting ranges of d0, z0 and d0Sig, z0Sig"
                << std::endl;
    }
    f->cd("/");
    // get run directory name
    TIter nextcd0(gDirectory->GetListOfKeys());
    TKey* key0 = (TKey*) nextcd0();
    TDirectory* dir0 = dynamic_cast<TDirectory*> (key0->ReadObj());

    if (dir0 != 0) {
      dir0->cd();

      TIter next_run(f->GetListOfKeys());
      TKey* key_run(0);

      // loop through runs in the file
      while ((key_run = dynamic_cast<TKey*> (next_run())) != 0) {
        if (debugLevel > 1) {
          std::cout << "--> BJetTaggingAdjustRanges: Getting run " << key_run << std::endl;
        }
        TObject* obj_run = key_run->ReadObj();
        TDirectory* tdir_run = dynamic_cast<TDirectory*> (obj_run);

        // Some checks of the run number
        if (tdir_run != 0) {
          //if (debugLevel > 0) {
          //	std::cerr << "--> BJetTaggingAdjustRanges: directory " << tdir_run
          //	<< " not found." << std::endl;
          //}
          //continue;
          //delete obj_run;

          std::string run_dir = tdir_run->GetName();
          if (run_dir.find("run") == std::string::npos) {
            std::cerr << "--> BJetTaggingAdjustRanges: no run found" << std::endl;
            return;
            //delete obj_run;
          }

          if (debugLevel > 1) {
            std::cout << "--> BJetTaggingAdjustRanges: Getting run no. " << run_dir
                      << std::endl;
          }

          // Setting the branch for the impact parameters
          std::string jetmon_dir = run_dir + "/JetTagging";

          TDirectory* dir(0);
          //std::cout << "--> BJetTaggingAdjustRanges: directory " << diag_dir
          //				<< std::endl;

          if (!(dir = f->GetDirectory(jetmon_dir.c_str()))) {
            if (debugLevel > 0) std::cerr << "--> BJetTaggingAdjustRanges: directory " << jetmon_dir
                                          << " not found." << std::endl;
            return;
          }
          // std::cout << "--> BJetTaggingAdjustRanges: Setting d0,z0,d0Sig,z0Sig vectors" << std::endl;
          static const std::vector<std::string> impactParameters = {
            "d0Sig",
            "z0Sig",
            "d0",
            "z0",
          };

          static const std::vector<std::string> etaAndPtRanges = {
            "_EtaRange_0_PtRange_0",
            "_EtaRange_0_PtRange_1",
            "_EtaRange_0_PtRange_2",
            "_EtaRange_1_PtRange_0",
            "_EtaRange_1_PtRange_1",
            "_EtaRange_1_PtRange_2",
            "_EtaRange_2_PtRange_0",
            "_EtaRange_2_PtRange_1",
            "_EtaRange_2_PtRange_2",
            "_EtaRange_3_PtRange_0",
            "_EtaRange_3_PtRange_1",
            "_EtaRange_3_PtRange_2",
            "_EtaRange_4_PtRange_0",
            "_EtaRange_4_PtRange_1",
            "_EtaRange_4_PtRange_2",
          };

          TH1F* workingHistogram(0);

          std::string impactParameterHistos, workingHistogramName;

          for (const std::string& impact : impactParameters) {
            for (const std::string& range : etaAndPtRanges) {
              workingHistogramName = impact + range;
              impactParameterHistos = jetmon_dir + "/" + workingHistogramName;

              workingHistogram = dynamic_cast<TH1F*> (f->Get(impactParameterHistos.c_str()));
              if (workingHistogram != 0) {
                if (debugLevel > 1) {
                  //std::cout << "--> BJetTaggingAdjustRanges: Really doing this stuff" <<
                  // workingHistogramName<<std::endl;
                }

                /*
                 * Here the bins are initialised and the upper and lower end from the histogram data
                 * are used as new borders of the updated histogram.
                 * */

                int minimalBin = 0;
                int maximalBin = 0;

                minimalBin = workingHistogram->FindFirstBinAbove(0.0, 1);
                maximalBin = workingHistogram->FindLastBinAbove(0.0, 1);

                workingHistogram->SetAxisRange(std::max(1, (minimalBin - 5)),
                                               (maximalBin + 5), "X");

                dir->cd();
                workingHistogram->Write("", TObject::kOverwrite);
              } else {
                if (debugLevel > 0) {
                  std::cerr << "Histogram " << impactParameterHistos << " not found."
                            << std::endl;
                }
              }
            } // for eta/Pt range
          } // for TrigItems
          if (debugLevel > 1) {
            std::cout << "--> BJetTaggingAdjustRanges: Finished" << std::endl;
          }
        }
      } // if tdir_run!=0
    }//while
  }//MonitoringFile::BJetTaggingAdjustRanges
}//namespace
