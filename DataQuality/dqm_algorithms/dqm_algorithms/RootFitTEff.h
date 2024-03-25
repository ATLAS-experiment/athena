/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/*! \file RootFitTEff.h file declares the dqm_algorithms::RootFitTEff  class.
 * \author Akimasa Ishikawa (akimasa.ishikawa@cern.ch) 15th Apr 2010 (original TGraph version)
 * \author Valerio Ippolito (valerio.ippolito@cern.ch) 14th Mar 2024 (adapted to TEfficiency)
*/

#ifndef DQM_ALGORITHMS_ROOTFITTEFF_H
#define DQM_ALGORITHMS_ROOTFITTEFF_H

#include "CxxUtils/checker_macros.h"

#include <dqm_core/Algorithm.h>
#include <string>
#include <memory>
#include <iosfwd>

class TF1;

namespace dqm_algorithms
{
  struct ATLAS_NOT_THREAD_SAFE RootFitTEff : public dqm_core::Algorithm
  //     ^ fit of constant graph
  {
    RootFitTEff( const std::string & name );
	  
    ~RootFitTEff();
    RootFitTEff * clone( );
    dqm_core::Result * execute( const std::string & , const TObject & , const dqm_core::AlgorithmConfig & );
    using dqm_core::Algorithm::printDescription;
    void  printDescription(std::ostream& out);             
    private:
    std::string m_name;
    std::unique_ptr<TF1> m_func;	
  };
}

#endif // DQM_ALGORITHMS_ROOTFITTEFF_H
