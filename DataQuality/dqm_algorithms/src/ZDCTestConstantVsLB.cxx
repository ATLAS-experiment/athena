/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "dqm_algorithms/ZDCTestConstantVsLB.h"


#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <TH2.h>
#include <TProfile.h>
#include <string>
#include "dqm_core/exceptions.h"
#include "dqm_core/AlgorithmManager.h"
#include "dqm_core/AlgorithmConfig.h"
#include "dqm_core/Result.h"
#include <ostream>
#include <TGraphErrors.h>
#include <TFitResult.h>

static dqm_algorithms::ZDCTestConstantVsLB staticInstance;

namespace dqm_algorithms {
	
	// *********************************************************************
	// Public Methods
	// *********************************************************************
	
	void
	ZDCTestConstantVsLB::
	printDescription(std::ostream& out)
	{
		std::string message;
		message += "\n";
		message += "Algorithm: \"" + m_name + "\"\n";
		message += "Description: Calculates the mean of the LB dependence\n";
		message += "A constant fit is performed, for bins with minimum statistics, and outliers are reported.  Yellow is for any 5 sigma deviations, red is for 10 or more\n";
		message += "             Overflow (and Underflow) bins are not included\n";
		message += "\n";
		
		out << message;
	}
	
	ZDCTestConstantVsLB::
	ZDCTestConstantVsLB()
    : m_name("ZDCTestConstantVsLB")
	{
		dqm_core::AlgorithmManager::instance().registerAlgorithm( m_name, this );
	}
	
	
	ZDCTestConstantVsLB::
	~ZDCTestConstantVsLB()
	{
	}
	
	
	dqm_core::Algorithm*
	ZDCTestConstantVsLB::
	clone()
	{
		return new ZDCTestConstantVsLB(*this);
	}
	
	
	dqm_core::Result*
	ZDCTestConstantVsLB::
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

	        TProfile *profile = histogram->ProfileX();
	        TH1 *projection = histogram->ProjectionX("projection");
                int Xbins = histogram->GetXaxis()->GetNbins();

		float minstat = dqm_algorithms::tools::GetFirstFromMap( "MinStat", config.getParameters(), 100);
		float NsigYellow = dqm_algorithms::tools::GetFirstFromMap( "NsigYellow", config.getParameters(), 5);
		float NsigMultRed = dqm_algorithms::tools::GetFirstFromMap( "NsigMultRed", config.getParameters(), 10);
				
                bool redflag = false;
                bool yellowflag = false;
                bool greenflag = true; // if nothing goes wrong, return green

		TGraphErrors* ge = new TGraphErrors;
		int nge =0;
		for (int ilb = 0; ilb<Xbins ;ilb++)
		  {
		    if (projection->GetBinContent(ilb+1) >= minstat)
		      {
			ge->SetPoint(nge,ilb,profile->GetBinContent(ilb+1));
			ge->SetPointError(nge,ilb,profile->GetBinError(ilb+1));
			nge++;
		      }
		  }

		TFitResultPtr fit_res = ge->Fit("pol0","QNS0");
		if (!fit_res.Get())
		  {
		    result->status_ = dqm_core::Result::Yellow;
		    return result;
		  }
		
		double mean = fit_res->Value(0);

		std::vector<int> yellowLBs;
		std::vector<double> yellowR;
		for (int ip = 0; ip<ge->GetN() ;ip++)
		  {
		    double x,y;
		    ge->GetPoint(ip,x,y);
		    double ey = ge->GetErrorY(ip);
		    double r = TMath::Abs(( y - mean ) / ey); // number of sigma from const mean, use only pointwise error
		    if (r>NsigYellow)
		      {
			yellowflag = true;
			yellowLBs.push_back(x); // store LB
			yellowR.push_back(r); // store R
		      }
		  }

		for (size_t iy=0;iy<yellowLBs.size();iy++)
		  {
		    int LB = yellowLBs.at(iy);
		    double R = yellowR.at(iy);
		    char ctag[256];
		    sprintf(ctag,"Bad LB %d with R", LB);
		    result->tags_[ctag] = R;
		  }
		
		if (yellowLBs.size() > NsigMultRed)
		  {
		    redflag = true;
		  }
		
		if (redflag)
		  {
		    result->status_ = dqm_core::Result::Red;
		  }
		else if (yellowflag)
		  {
		    result->status_ = dqm_core::Result::Yellow;
		  }
		else if (greenflag)
		  {
		    result->status_ = dqm_core::Result::Green;
		  }

		return result;
	}
	
}
