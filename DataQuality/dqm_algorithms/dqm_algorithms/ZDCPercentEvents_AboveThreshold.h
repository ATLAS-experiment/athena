/*
		Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*! \file ZDCPercentEvents_AboveThreshold.h file declares the dqm_algorithms::algorithm::ZDCPercentEvents_AboveThreshold class.
 * \author Yuhan Guo
*/

#ifndef DQM_ALGORITHMS_ZDCPERCENTEVENTS_ABOVETHRESHOLD_H
#define DQM_ALGORITHMS_ZDCPERCENTEVENTS_ABOVETHRESHOLD_H

#include <dqm_algorithms/ZDCPercentageThreshTests.h>

namespace dqm_algorithms{
	struct ZDCPercentEvents_AboveThreshold : public ZDCPercentageThreshTests{
	  	ZDCPercentEvents_AboveThreshold():  ZDCPercentageThreshTests("AboveThreshold")  {};
	};
}

#endif // DQM_ALGORITHMS_ZDCPERCENTEVENTS_ABOVETHRESHOLD_H
