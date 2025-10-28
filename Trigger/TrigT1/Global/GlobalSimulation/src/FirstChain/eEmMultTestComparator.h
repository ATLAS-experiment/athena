/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMMULTTESTCOMPARATOR_H
#define GLOBALSIM_EEMMULTTESTCOMPARATOR_H

/**
 * AlgTool that to test whether expected the TIP values generated
 * by data supplied by eEmMultTestBench correspond to those
 * produced by eEmMultAlgTool
 */

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "../ITIPwriterAlgTool.h" // TIP word declaration

#include "../IO/TipWord_clid.h"

#include "AthenaBaseComps/AthAlgTool.h"

namespace GlobalSim {
  class eEmMultTestComparator: public AthReentrantAlgorithm {
    
  public:

    
    eEmMultTestComparator(const std::string& name, ISvcLocator *pSvcLocator);
    
    virtual ~eEmMultTestComparator() = default;
    
    virtual StatusCode initialize() override;

    virtual StatusCode execute(const EventContext& ctx) const override;
    
  private:

    
    SG::ReadHandleKey<TIPword>
    m_expectedTIPword_ReadKey {
      this,
	"ExpectedTIPwordReadKey",
	"ExpectedTIPwords",
	"key to read in expected TIP words"
	};

        
    SG::ReadHandleKey<TIPword>
    m_generatedTIPword_ReadKey {
      this,
      "GeneratedTIPwordReadKey",
      "GlobalSimTIP",
      "key to read in GlobalSim TIP words"
    };
 
    Gaudi::Property<bool>
    m_abort_on_mismatch{this,
			"abort_on_mismatch",
			{false},
			"falg to abort on first exp, gen TIP word mismatch"};
    
  };
}
#endif
