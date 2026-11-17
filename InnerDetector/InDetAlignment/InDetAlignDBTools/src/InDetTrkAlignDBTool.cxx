/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "InDetAlignDBTools/InDetTrkAlignDBTool.h"

#include "TFile.h"

#include <vector>

namespace InDet
{

//________________________________________________________________________
InDetTrkAlignDBTool::~InDetTrkAlignDBTool()
{}

//________________________________________________________________________
StatusCode InDetTrkAlignDBTool::initialize() {

  ATH_MSG_DEBUG("initialize() of InDetTrkAlignDBTool");

  // get SiTrkAlignDBTool
  if( m_siDBtool.retrieve().isFailure() ) {
    msg(MSG::FATAL) << " Could not retrieve " << m_siDBtool << endmsg;
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("retrieved " << m_siDBtool);

  // get TRTTrkAlignDBTool
  if( m_trtDBtool.retrieve().isFailure() ) {
    msg(MSG::FATAL) << " Could not retrieve " << m_trtDBtool << endmsg;
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("retrieved " << m_trtDBtool);

  return StatusCode::SUCCESS;
}

//________________________________________________________________________
StatusCode InDetTrkAlignDBTool::finalize()
{
  ATH_MSG_DEBUG("finalize() of InDetTrkAlignDBTool");

  return StatusCode::SUCCESS;
}

//________________________________________________________________________
void InDetTrkAlignDBTool::writeAlignPar()
{
  ATH_MSG_DEBUG("in InDetTrkAlignDBTool::writeAlignPar()");

  m_siDBtool->writeAlignPar();
  m_trtDBtool->writeAlignPar();

  return;
}
//_________________________________________________________________________________________

} // end namespace
