/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*! \file TileTriggerMonitor.h file declares the dqm_algorithms::TileTriggerMonitor  class.
 * \author Kyle Lleras
*/

#ifndef DQM_ALGORITHMS_TileTriggerMonitor_H
#define DQM_ALGORITHMS_TileTriggerMonitor_H

#include <dqm_core/Algorithm.h>
#include <string>
#include <iosfwd>

namespace dqm_algorithms
{
	struct TileTriggerMonitor : public dqm_core::Algorithm
        {
	  TileTriggerMonitor();

	    //overwrites virtual functions
	  TileTriggerMonitor * clone( );
	  dqm_core::Result * execute( const std::string & , const TObject & , const dqm_core::AlgorithmConfig & );
          using dqm_core::Algorithm::printDescription;
	  void  printDescription(std::ostream& out);

	};
}

#endif // DQM_ALGORITHMS_TileTriggerMonitor_H
