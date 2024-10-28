/*
		Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*! \file ZDCPercentEvents_UnderThreshold.h file declares the dqm_algorithms::algorithm::ZDCPercentEvents_UnderThreshold class.
 * \author Yuhan Guo
*/

#ifndef DQM_ALGORITHMS_ZDCPERCENTEVENTS_UNDERTHRESHOLD_H
#define DQM_ALGORITHMS_ZDCPERCENTEVENTS_UNDERTHRESHOLD_H

#include <dqm_algorithms/ZDCPercentageThreshTests.h>

namespace dqm_algorithms{
	struct ZDCPercentEvents_UnderThreshold : public ZDCPercentageThreshTests{
	  	ZDCPercentEvents_UnderThreshold():  ZDCPercentageThreshTests("UnderThreshold")  {};
	};
}

#endif // DQM_ALGORITHMS_ZDCPERCENTEVENTS_UNDERTHRESHOLD_H
