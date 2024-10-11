/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArG4RunControl/LArGeoTBGeometricOptions.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Bootstrap.h"
#include "StoreGate/StoreGateSvc.h"

void LArGeoTBGeometricOptions::saveMe()
{
  SmartIF<StoreGateSvc> detStore{Gaudi::svcLocator()->service("DetectorStore")};
  if (!detStore) {
    std::cout << "LArGeoTBGeometricOptions::saveMe ERROR Could not dynamic cast det store" << std::endl;
    return;
  }
  if(!detStore->record(this,"LArGeoTBGeometricOptions").isSuccess()) {
    std::cout << "Can not record LArGeoTBGeometricOptions" << std::endl;
  }
}

void LArGeoTBGeometricOptions::printMe()
{
  std::cout << " *** *** This is the object of type LArGeoTBGeometricOptions *** *** \n";
  std::cout << " ** CryoEtaPosition = " << m_CryoEtaPosition << "\n *** *** \n";
  std::cout << " ** CryoPhiPosition = " << m_CryoPhiPosition << "\n *** *** \n";
}
