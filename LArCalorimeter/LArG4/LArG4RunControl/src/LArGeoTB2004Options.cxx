/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArG4RunControl/LArGeoTB2004Options.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Bootstrap.h"
#include "StoreGate/StoreGateSvc.h"

void LArGeoTB2004Options::saveMe()
{
  SmartIF<StoreGateSvc> detStore{Gaudi::svcLocator()->service("DetectorStore")};
  if (!detStore) {
    std::cout << "LArGeoTB2004Options::saveMe ERROR Could not dynamic cast det store" << std::endl;
    return;
  }
  if (!detStore->record(this,"LArGeoTB2004Options").isSuccess()) {
    std::cout << "Can not record LArGeoTB2004Options" << std::endl;
  }
}

void LArGeoTB2004Options::printMe() const
{
  std::cout << " *** *** This is the object of type LArGeoTB2004Options *** *** \n";
  std::cout << " ** H1 Cryostat X Position = " << m_CryoXPosition << "\n *** *** \n";
  std::cout << " ** H1 Table Y Position    = " << m_TableYPosition << "\n *** *** \n";
  std::cout << " ** Run1                   = " << m_Run1 << "\n *** *** \n";
  std::cout << " ** Emec                   = " << m_emec << "\n *** *** \n";
  std::cout << " ** Hec                    = " << m_hec << "\n *** *** \n";
  std::cout << " ** Fcal                   = " << m_fcal << "\n *** *** \n";
  std::cout << " ** Coldnose               = " << m_coldnose << "\n *** *** \n";
  std::cout << " ** Print step             = " << m_printstep << "\n *** *** \n";
  std::cout << " ** Check primary          = " << m_checkprim << "\n *** *** \n";
  std::cout << " ** Check other            = " << m_checkother << "\n *** *** \n";
}
