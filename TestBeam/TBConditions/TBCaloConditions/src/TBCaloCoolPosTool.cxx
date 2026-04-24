/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TBCaloCoolPosTool.h"


// Gaudi includes
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/ServiceHandle.h"


//--------------------------------------------------------------------------
StatusCode TBCaloCoolPosTool::initialize()
{
    ATH_CHECK( m_etaTableKey.initialize() );
    ATH_CHECK( m_thetaTableKey.initialize() );
    ATH_CHECK( m_zTableKey.initialize() );
    ATH_CHECK( m_deltaTableKey.initialize() );
    return StatusCode::SUCCESS;
}


double TBCaloCoolPosTool::eta() const
{
  SG::ReadCondHandle<AthenaAttributeList> eta (m_etaTableKey);
  return (**eta)["eta"].data<float>();
}

double TBCaloCoolPosTool::theta() const
{
  SG::ReadCondHandle<AthenaAttributeList> theta (m_thetaTableKey);
  return (**theta)["theta"].data<float>();
}

double TBCaloCoolPosTool::z() const
{
  SG::ReadCondHandle<AthenaAttributeList> z (m_zTableKey);
  return (**z)["z"].data<float>();
}

double TBCaloCoolPosTool::delta() const
{
  SG::ReadCondHandle<AthenaAttributeList> delta (m_deltaTableKey);
  return (**delta)["delta"].data<float>();
}

