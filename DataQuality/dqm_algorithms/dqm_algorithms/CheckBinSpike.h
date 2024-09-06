/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*! \file CheckBinSpike.h file declares the dqm_algorithms::CheckBinSpike  class.
 * \author Juan L. Marin and Takuya Nobe
*/

#ifndef DQM_ALGORITHMS_CHECKBINSPIKE_H
#define DQM_ALGORITHMS_CHECKBINSPIKE_H

#include <dqm_core/Algorithm.h>
#include <TH1.h>


namespace dqm_algorithms
{
	struct CheckBinSpike : public dqm_core::Algorithm
    {
	    CheckBinSpike(const std::string & name);
		CheckBinSpike * clone( );
		dqm_core::Result * execute( const std::string & , const TObject & , const dqm_core::AlgorithmConfig & );
		using dqm_core::Algorithm::printDescription;
		void  printDescription(std::ostream& out);
		void checkBinByBin(const TH1* hist, float &result,  float &detectedBin);
		private:
			std::string m_name;

              
	};

}

#endif // DQM_ALGORITHMS_CHECKBINSPIKE_H
