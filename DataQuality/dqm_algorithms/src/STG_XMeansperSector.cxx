/*
  Copyright (C) 2002-2025 CERN
  for the benefit of the ATLAS collaboration
*/

#include "dqm_algorithms/STG_XMeansperSector.h"

#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <TH2.h>
#include <TProfile.h>
#include <string>
#include <cmath>
#include <iostream>

#include "dqm_core/exceptions.h"
#include "dqm_core/AlgorithmManager.h"
#include "dqm_core/AlgorithmConfig.h"
#include "dqm_core/Result.h"

static dqm_algorithms::STG_XMeansperSector staticInstance;

namespace dqm_algorithms {

  // *********************************************************************
  // Constructor
  // *********************************************************************
  STG_XMeansperSector::STG_XMeansperSector() {
    dqm_core::AlgorithmManager::instance().registerAlgorithm(m_name, this);
  }

  // *********************************************************************
  // Clone
  // *********************************************************************
  dqm_core::Algorithm*
  STG_XMeansperSector::clone() {
    return new STG_XMeansperSector(*this);
  }

  // *********************************************************************
  // Print Description
  // *********************************************************************
  void
  STG_XMeansperSector::printDescription(std::ostream& out) {
    out << "\n";
    out << "Algorithm: \"" << m_name << "\"\n";
    out << "Description: Evaluates <X> means per Y-bin in STGC timing histograms.\n";
    out << "Flags sectors as RED if <X> deviates beyond redMean.\n";
    out << "Overflow and underflow bins are excluded.\n";
    out << "\n";
  }

  // *********************************************************************
  // Execute
  // *********************************************************************
  dqm_core::Result*
  STG_XMeansperSector::
  execute( const std::string& name, const TObject& object, const dqm_core::AlgorithmConfig& config)
  {
    //No status flags are set
    dqm_core::Result* result = new dqm_core::Result();
    result->status_ = dqm_core::Result::Undefined;
          const TH2 * histogram;
  
                if( object.IsA()->InheritsFrom( "TH2" ) ) {
                 histogram = static_cast<const TH2*>(&object);
                 if (histogram->GetDimension() > 2 ){ 
                  throw dqm_core::BadConfig( ERS_HERE, name, "dimension > 2 " );
                 }
                } else {
                   throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH2" );
                }

                TProfile *h2 = histogram->ProfileY();
                int Xbins = histogram->GetXaxis()->GetNbins();
                int Ybins = histogram->GetYaxis()->GetNbins();

                double mean_global = histogram->GetMean(1);

                float Meanlow;
                float Meanhigh;
                float redMean;
                if (Xbins > 100) {
                 Meanlow = dqm_algorithms::tools::GetFirstFromMap( "MeanL", config.getParameters(), mean_global-12.5);
                 Meanhigh = dqm_algorithms::tools::GetFirstFromMap( "MeanH", config.getParameters(), mean_global+12.5);
                 redMean = dqm_algorithms::tools::GetFirstFromMap( "MeanRed", config.getParameters(), mean_global+30.0);     
                } else if (Xbins < 15) {
                 Meanlow = dqm_algorithms::tools::GetFirstFromMap( "MeanL", config.getParameters(), mean_global-50.0);
                 Meanhigh = dqm_algorithms::tools::GetFirstFromMap( "MeanH", config.getParameters(), mean_global+50.0);
                 redMean = dqm_algorithms::tools::GetFirstFromMap( "MeanRed", config.getParameters(), mean_global+75.0);
                } else {
                 Meanlow = dqm_algorithms::tools::GetFirstFromMap( "MeanL", config.getParameters(), mean_global-25.0);
                 Meanhigh = dqm_algorithms::tools::GetFirstFromMap( "MeanH", config.getParameters(), mean_global+25.0);
                 redMean = dqm_algorithms::tools::GetFirstFromMap( "MeanRed", config.getParameters(), mean_global+50.0);
                } 

                std::vector<float> MeanX(Ybins, 0.0);
                //float MeanX[Ybins];
                bool redflag = false;
                bool yellowflag = false;
                bool greenflag = false;
                int Passed=0;
                for (int i = 1; i <= Ybins; i++) {
                 MeanX[i]=h2->GetBinContent(i);
                 if (MeanX[i]==0) {
                    Passed = Passed +1;
                    continue;
                 }
                 if (MeanX[i] > Meanlow && MeanX[i] < Meanhigh) Passed = Passed +1;
                 if (abs(MeanX[i]-mean_global) > abs(redMean-mean_global))  redflag=true;
                }
                double gthreshold;
                double rthreshold;
                try {
                 rthreshold = dqm_algorithms::tools::GetFromMap("OOBSectors", config.getRedThresholds());
                 gthreshold = dqm_algorithms::tools::GetFromMap("OOBSectors", config.getGreenThresholds());
                } 
                catch ( dqm_core::Exception & ex ) {
                 throw dqm_core::BadConfig( ERS_HERE, name, ex.what(), ex );
                }      
                if (Passed >= (Ybins-gthreshold) && not redflag) greenflag=true;
                else if (Passed >= (Ybins-rthreshold) && not redflag) yellowflag=true;
                else redflag=true;
                if ( greenflag ) {
                 result->status_ = dqm_core::Result::Green;
                } else if ( yellowflag ) {
                  result->status_ = dqm_core::Result::Yellow;
                } else {
                  result->status_ = dqm_core::Result::Red;
                }
                result->tags_["Xbins"] = Xbins;
                result->tags_["Passed"] = Passed;
                result->tags_["MeanGlobal"] = mean_global;
    return result;
  }

} // namespace dqm_algorithms

