
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eEmMultTestComparator.h"

#include <sstream>
#include <algorithm>

namespace GlobalSim {

  eEmMultTestComparator::eEmMultTestComparator(const std::string& name,
					   ISvcLocator *pSvcLocator):
    AthReentrantAlgorithm(name, pSvcLocator) {
  }
  
  StatusCode eEmMultTestComparator::initialize() {
       
    CHECK(m_expectedTIPword_ReadKey.initialize());
    CHECK(m_generatedTIPword_ReadKey.initialize());

    ATH_MSG_INFO(m_expectedTIPword_ReadKey);
    ATH_MSG_INFO(m_generatedTIPword_ReadKey);


    return StatusCode::SUCCESS;
  }
  
  StatusCode
  eEmMultTestComparator::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("run()");

  
    // read in the expected and the generated TIP

    auto expectedTIP =
      SG::ReadHandle<GlobalSim::TIPword>( m_expectedTIPword_ReadKey, ctx);
    CHECK(expectedTIP.isValid());
     
    ATH_MSG_DEBUG("read in the expected TIP word");


    auto generatedTIP =
      SG::ReadHandle<GlobalSim::TIPword>(m_generatedTIPword_ReadKey, ctx);
    CHECK(generatedTIP.isValid());
     
    ATH_MSG_DEBUG("read in the generated TIP word");

    auto esz = (*expectedTIP).size();
    auto gsz = (*generatedTIP).size();

    if (esz != gsz) {
      ATH_MSG_ERROR("expected TIP and generated TIP sizes differ "
		    <<esz << ' ' << gsz);
      return StatusCode::FAILURE;
    }

    if (*generatedTIP ==  *expectedTIP) {
      ATH_MSG_DEBUG("Test passed");
      return StatusCode::SUCCESS;
    }


    ATH_MSG_INFO("Expected, generated TIP word mismatch.  Event "
		 << ctx.evt());
    
    for (std::size_t i{0}; i != esz; ++i) {
      if (generatedTIP->test(i) != expectedTIP->test(i)) {
	ATH_MSG_INFO("TIP word postion " << i << std::boolalpha
		     << " expected " << expectedTIP->test(i) 
		     << " generated " << generatedTIP->test(i));
      }
    }

    if (m_abort_on_mismatch) {return StatusCode::FAILURE;}

    return StatusCode::SUCCESS;
  }

}

