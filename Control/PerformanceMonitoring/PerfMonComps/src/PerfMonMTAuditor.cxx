/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Framework includes
#include "AthenaBaseComps/AthCheckMacros.h"

// PerfMonKernel includes
#include "PerfMonKernel/IPerfMonMTSvc.h"

// PerfMonComps includes
#include "PerfMonMTAuditor.h"


/*
 * Constructor
 */
PerfMonMTAuditor::PerfMonMTAuditor( const std::string& name,
                                    ISvcLocator* pSvcLocator ) :
  Auditor ( name, pSvcLocator  ),
  m_perfMonMTSvc ( "PerfMonMTSvc", name )
{
}

/*
 * Initialize the Auditor
 */
StatusCode PerfMonMTAuditor::initialize()
{
  ATH_CHECK( m_perfMonMTSvc.retrieve() );

  return StatusCode::SUCCESS;
}

/*
 * Implementation of base class methods
 */
void PerfMonMTAuditor::before(const std::string& event, const std::string& name,
                              const EventContext& ctx) {
  m_perfMonMTSvc->startAud( event , name , ctx );
}

void PerfMonMTAuditor::after(const std::string& event, const std::string& name,
                             const EventContext& ctx, const StatusCode&) {
  m_perfMonMTSvc->stopAud( event , name , ctx);
}
