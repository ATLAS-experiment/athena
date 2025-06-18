/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArG4H62004InactiveSDTool.h"

#include "LArG4H62004CalibSD.h"

LArG4H62004InactiveSDTool::LArG4H62004InactiveSDTool(const std::string& type, const std::string& name, const IInterface *parent)
  : LArG4SDTool(type,name,parent)
  , m_HitColl("LArCalibrationHitInactive")
{
}

StatusCode LArG4H62004InactiveSDTool::initializeCalculators()
{
  ATH_CHECK(m_emepiwcalc.retrieve());
  ATH_CHECK(m_heccalc.retrieve());
  ATH_CHECK(m_fcal1calc.retrieve());
  ATH_CHECK(m_fcal2calc.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode LArG4H62004InactiveSDTool::initializeSD()
{
  // Setup calculator and collection
  if (!m_emecVolumes.value().empty()) m_emecSD  = new LArG4H62004CalibSD( "LAr::EMEC::InnerModule::Inactive::H6" , &*m_emepiwcalc , m_doPID );
  if (!m_hecVolumes.value().empty())  m_hecSD  = new LArG4H62004CalibSD( "LAr::HEC::Local::Inactive::H6" , &*m_heccalc , m_doPID );
  if (!m_fcal1Volumes.value().empty()) m_fcal1SD  = new LArG4H62004CalibSD( "LAr::FCAL::Inactive1::H6" , &*m_fcal1calc , m_doPID );
  if (!m_fcal2Volumes.value().empty()) m_fcal2SD  = new LArG4H62004CalibSD( "LAr::FCAL::Inactive2::H6" , &*m_fcal2calc , m_doPID );

  std::map<G4VSensitiveDetector*,std::vector<std::string>*> configuration;
  if (!m_emecVolumes.value().empty()) configuration[m_emecSD]  = &m_emecVolumes.value();
  if (!m_hecVolumes.value().empty())  configuration[m_hecSD]   = &m_hecVolumes.value();
  if (!m_fcal1Volumes.value().empty()) configuration[m_fcal1SD]  = &m_fcal1Volumes.value();
  if (!m_fcal2Volumes.value().empty()) configuration[m_fcal2SD]  = &m_fcal2Volumes.value();
  setupAllSDs(configuration);

  // make sure they have the identifiers they need
  if (!m_emecVolumes.value().empty()) setupHelpers(m_emecSD);
  if (!m_hecVolumes.value().empty())  setupHelpers(m_hecSD);
  if (!m_fcal1Volumes.value().empty()) setupHelpers(m_fcal1SD);
  if (!m_fcal2Volumes.value().empty()) setupHelpers(m_fcal2SD);

  return StatusCode::SUCCESS;
}

StatusCode LArG4H62004InactiveSDTool::Gather()
{
  // In this case, *unlike* other SDs, the *tool* owns the collection
  if (!m_HitColl.isValid()) m_HitColl = std::make_unique<CaloCalibrationHitContainer>(m_HitColl.name());
  if (!m_emecVolumes.value().empty()) m_emecSD ->EndOfAthenaEvent( &*m_HitColl );
  if (!m_hecVolumes.value().empty())  m_hecSD  ->EndOfAthenaEvent( &*m_HitColl );
  if (!m_fcal1Volumes.value().empty()) m_fcal1SD ->EndOfAthenaEvent( &*m_HitColl );
  if (!m_fcal2Volumes.value().empty()) m_fcal2SD ->EndOfAthenaEvent( &*m_HitColl );
  return StatusCode::SUCCESS;
}
