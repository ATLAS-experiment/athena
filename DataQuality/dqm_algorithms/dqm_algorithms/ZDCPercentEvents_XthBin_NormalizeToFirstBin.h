/*
		Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*! \file ZDCPercentEvents_XthBin_NormalizeToFirstBin.h file declares the dqm_algorithms::algorithm::ZDCPercentEvents_XthBin_NormalizeToFirstBin class.
 * \author Yuhan Guo
*/

#ifndef DQM_ALGORITHMS_ZDCPERCENTEVENTS_XTHBIN_NORMALIZETOFIRSTBIN_H
#define DQM_ALGORITHMS_ZDCPERCENTEVENTS_XTHBIN_NORMALIZETOFIRSTBIN_H

#include <dqm_algorithms/ZDCPercentageThreshTests.h>

namespace dqm_algorithms{
	struct ZDCPercentEvents_XthBin_NormalizeToFirstBin : public ZDCPercentageThreshTests{
	  	ZDCPercentEvents_XthBin_NormalizeToFirstBin():  ZDCPercentageThreshTests("XthBin_NormalizeToFirstBin")  {};
	};
}

#endif // DQM_ALGORITHMS_ZDCPERCENTEVENTS_XTHBIN_H
