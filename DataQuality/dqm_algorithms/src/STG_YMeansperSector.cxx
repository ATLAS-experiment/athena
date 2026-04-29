/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "dqm_algorithms/STG_YMeansperSector.h"


#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <TH2.h>
#include <TProfile.h>
#include <string>
#include "dqm_core/exceptions.h"
#include "dqm_core/AlgorithmManager.h"
#include "dqm_core/AlgorithmConfig.h"
#include "dqm_core/Result.h"
#include <ostream>

static dqm_algorithms::STG_YMeansperSector staticInstance;

namespace dqm_algorithms {
	
	// *********************************************************************
	// Public Methods
	// *********************************************************************
	
	void
	STG_YMeansperSector::
	printDescription(std::ostream& out)
	{
		std::string message;
		message += "\n";
		message += "Algorithm: \"" + m_name + "\"\n";
		message += "Description: Checks the Ymeans per sector of every histogram\n";
		message += "If the Ymeans are outside the range the sector is flagged red. If one of the sectors is very far from the Ymean the whole histogram is flagged red\n";
		message += "             Overflow (and Underflow) bins are not included\n";
		message += "\n";
		
		out << message;
	}
	
	STG_YMeansperSector::
	STG_YMeansperSector()
    : m_name("STG_YMeansperSector")
	{
		dqm_core::AlgorithmManager::instance().registerAlgorithm( m_name, this );
	}
	
	
	STG_YMeansperSector::
	~STG_YMeansperSector()
	{
	}
	
	
	dqm_core::Algorithm*
	STG_YMeansperSector::
	clone()
	{
		return new STG_YMeansperSector(*this);
	}
	
	
	dqm_core::Result*
	STG_YMeansperSector::
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
	        TProfile *h2 = histogram->ProfileX();
                int Xbins = histogram->GetXaxis()->GetNbins();
                int Ybins = histogram->GetYaxis()->GetNbins();

                float Meanlow = 0;
                float Meanhigh = 0.0;
                float redMean = 0.0;
                if (Ybins < 15) {
                 Meanlow = dqm_algorithms::tools::GetFirstFromMap( "MeanL", config.getParameters(), -85.5);
                 Meanhigh = dqm_algorithms::tools::GetFirstFromMap( "MeanH", config.getParameters(), 85.5);
                 redMean = dqm_algorithms::tools::GetFirstFromMap( "MeanRed", config.getParameters(), 125.0);
                 
                 
                } else if (Ybins > 100) {
                 Meanlow = dqm_algorithms::tools::GetFirstFromMap( "MeanL", config.getParameters(), -150.5);
                 Meanhigh = dqm_algorithms::tools::GetFirstFromMap( "MeanH", config.getParameters(), 150.5);
                 redMean = dqm_algorithms::tools::GetFirstFromMap( "MeanRed", config.getParameters(), 200.0);
                }
		else {
                	Meanlow = dqm_algorithms::tools::GetFirstFromMap( "MeanL", config.getParameters(), -115.5);
                	Meanhigh = dqm_algorithms::tools::GetFirstFromMap( "MeanH", config.getParameters(), 115.5);
                	redMean = dqm_algorithms::tools::GetFirstFromMap( "MeanRed", config.getParameters(),150.0);
                } 

	        std::vector<float> MeanY(Xbins, 0.0);
                bool redflag = false;
                bool yellowflag = false;
                bool greenflag = false;
                int Passed=0;
                for (int i = 1; i <= Xbins; i++) {
                 MeanY[i]=h2->GetBinContent(i);
                 if (MeanY[i] > Meanlow && MeanY[i] < Meanhigh) Passed = Passed +1;
                 if (abs(MeanY[i]) > abs(redMean))  redflag=true;
                }
                double gthreshold;
                double rthreshold;
                try {
                 rthreshold = dqm_algorithms::tools::GetFromMap( "NSectors", config.getRedThresholds() );
                 gthreshold = dqm_algorithms::tools::GetFromMap( "NSectors", config.getGreenThresholds() );
                } 
                catch ( dqm_core::Exception & ex ) {
                 throw dqm_core::BadConfig( ERS_HERE, name, ex.what(), ex );
                }
                if (Passed > gthreshold && not redflag) greenflag=true;
                else if (Passed > rthreshold && not redflag) yellowflag=true;
                else redflag=true;
                if ( greenflag ) {
                 result->status_ = dqm_core::Result::Green;
                } else if ( yellowflag ) {
                  result->status_ = dqm_core::Result::Yellow;
                } else {
                  result->status_ = dqm_core::Result::Red;
                }
		
		return result;
	}
	
}
