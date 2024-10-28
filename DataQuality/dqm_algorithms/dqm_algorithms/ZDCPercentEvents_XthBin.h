/*
		Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*! \file ZDCPercentEvents_XthBin.h file declares the dqm_algorithms::algorithm::ZDCPercentEvents_XthBin class.
 * \author Yuhan Guo
*/

#ifndef DQM_ALGORITHMS_ZDCPERCENTEVENTS_XTHBIN_H
#define DQM_ALGORITHMS_ZDCPERCENTEVENTS_XTHBIN_H

#include <dqm_algorithms/ZDCPercentageThreshTests.h>

namespace dqm_algorithms{
	struct ZDCPercentEvents_XthBin : public ZDCPercentageThreshTests{
	  	ZDCPercentEvents_XthBin():  ZDCPercentageThreshTests("XthBin")  {};
	};
}

#endif // DQM_ALGORITHMS_ZDCPERCENTEVENTS_XTHBIN_H
