/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArFastShowerTool.h"
#include "BarrelFastSimDedicatedSD.h"
#include "EndcapFastSimDedicatedSD.h"
#include "FCALFastSimDedicatedSD.h"
#include "LArFastShower.h"
#include "G4SDManager.hh"

LArFastShowerTool::LArFastShowerTool(const std::string& type
				     , const std::string& name
                                     , const IInterface *parent)
  : FastSimulationBase(type, name, parent)
{
}

StatusCode LArFastShowerTool::initialize()
{
  ATH_MSG_VERBOSE( name() << "::initialize()" );
  CHECK( m_showerLibSvc.retrieve() );

  m_configuration.m_showerLibSvcName = m_showerLibSvc.name();
  
  m_configuration.m_e_FlagShowerLib = m_e_FlagShowerLib;
  m_configuration.m_e_MinEneShowerLib = m_e_MinEneShowerLib;
  m_configuration.m_e_MaxEneShowerLib = m_e_MaxEneShowerLib;

  m_configuration.m_g_FlagShowerLib = m_g_FlagShowerLib;
  m_configuration.m_g_MinEneShowerLib = m_g_MinEneShowerLib;
  m_configuration.m_g_MaxEneShowerLib = m_g_MaxEneShowerLib;

  m_configuration.m_Neut_FlagShowerLib = m_Neut_FlagShowerLib;
  m_configuration.m_Neut_MinEneShowerLib = m_Neut_MinEneShowerLib;
  m_configuration.m_Neut_MaxEneShowerLib = m_Neut_MaxEneShowerLib;

  m_configuration.m_Pion_FlagShowerLib = m_Pion_FlagShowerLib;
  m_configuration.m_Pion_MinEneShowerLib = m_Pion_MinEneShowerLib;
  m_configuration.m_Pion_MaxEneShowerLib = m_Pion_MaxEneShowerLib;

  m_configuration.m_containLow = m_containLow;
  m_configuration.m_absLowEta = m_absLowEta;
  m_configuration.m_containHigh = m_containHigh;
  m_configuration.m_absHighEta = m_absHighEta;
  m_configuration.m_containCrack = m_containCrack;
  m_configuration.m_absCrackEta1 = m_absCrackEta1;
  m_configuration.m_absCrackEta2 = m_absCrackEta2;

  m_configuration.m_generated_starting_points_file = m_generated_starting_points_file;
  m_configuration.m_generated_starting_points_ratio = m_generated_starting_points_ratio;
  m_configuration.m_detector_tag = m_detector_tag;
  m_configuration.m_applyRRWeights = m_applyRRWeights;

  return FastSimulationBase::initialize();
}

G4VFastSimulationModel* LArFastShowerTool::makeFastSimModel()
{
  ATH_MSG_DEBUG( "Initializing Fast Sim Model" );
  IFastSimDedicatedSD* fastSD = dynamic_cast<IFastSimDedicatedSD*>(G4SDManager::GetSDMpointer()->FindSensitiveDetector(m_fastSimDedicatedSD.value(), false));
  if (fastSD) {
    ATH_MSG_DEBUG( "SD " << m_fastSimDedicatedSD << " already created." );
  } else if ("BarrelFastSimDedicatedSD" == m_fastSimDedicatedSD){
    fastSD = new BarrelFastSimDedicatedSD( &*detStore(), msgLevel(MSG::DEBUG) );
  } else if ("EndcapFastSimDedicatedSD" == m_fastSimDedicatedSD){
    fastSD = new EndcapFastSimDedicatedSD( &*detStore(), msgLevel(MSG::DEBUG) );
  } else if ("FCALFastSimDedicatedSD" == m_fastSimDedicatedSD){
    fastSD = new FCALFastSimDedicatedSD( &*detStore(), msgLevel(MSG::DEBUG) );
  } else {
    ATH_MSG_FATAL( "Fast sim SD type " << m_fastSimDedicatedSD << " not found!" );
    throw std::runtime_error("Bad SD name");
  }
  G4SDManager::GetSDMpointer()->AddNewDetector(fastSD);

  // Create a fresh Fast Sim Model
  return new LArFastShower(name(), getRegion(), m_configuration, fastSD);
}
