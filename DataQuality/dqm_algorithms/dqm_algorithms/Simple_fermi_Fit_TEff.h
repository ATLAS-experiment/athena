/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/*! \file Simple_erf_Fit_TEff.h file declares the dqm_algorithms::algorithm::Simple_fermi_Fit_TEff class.  \n Format copied from Simple_gaus_Fit.h
 * \author Matt King and Akimasa Ishikawa
 * \author Valerio Ippolito
*/

#ifndef DQM_ALGORITHMS_SIMPLE_FERMI_FIT_TEFF_H
#define DQM_ALGORITHMS_SIMPLE_FERMI_FIT_TEFF_H

#include <dqm_algorithms/RootFitTEff.h>

namespace dqm_algorithms
{
	struct Simple_fermi_Fit_TEff : public RootFitTEff
        {
	     Simple_fermi_Fit_TEff():  RootFitTEff("fermi")  {};

              
	};

}

#endif // DQM_ALGORITHMS_SIMPLE_FERMI_FIT_TEFF_H
