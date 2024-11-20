/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DQM_ALGORITHMS_EFFICIENCYREFCOMP_H
#define DQM_ALGORITHMS_EFFICIENCYREFCOMP_H

#include <dqm_core/Algorithm.h>
#include <string>
#include <iosfwd>

namespace dqm_algorithms
{
	struct EfficiencyRefComp : public dqm_core::Algorithm
        {
	  EfficiencyRefComp();

	    //overwrites virtual functions
	  EfficiencyRefComp * clone( );
	  dqm_core::Result * execute( const std::string & , const TObject & , const dqm_core::AlgorithmConfig & );
          using dqm_core::Algorithm::printDescription;
	  void  printDescription(std::ostream& out);
};
}

#endif // DQM_ALGORITHMS_EFFICIENCYREFCOMP_H