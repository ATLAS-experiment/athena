/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArG4RunControl/LArGeoTBH1GeoOptions.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Bootstrap.h"
#include "StoreGate/StoreGateSvc.h"

void LArGeoTBH1GeoOptions::saveMe()
{
  SmartIF<StoreGateSvc> detStore{Gaudi::svcLocator()->service("DetectorStore")};
  if (!detStore) {
    std::cout << "LArGeoTBH1GeoOptions::saveMe ERROR Could not dynamic cast det store" << std::endl;
    return;
  }
  if (!detStore->record(this,"LArGeoTBH1GeoOptions").isSuccess()) {
    std::cout << "Can not record LArGeoTBH1GeoOptions" << std::endl;
  }
}

void LArGeoTBH1GeoOptions::printMe()
{
  std::cout << " *** *** This is the object of type LArGeoTBH1GeoOptions *** *** \n";
  std::cout << " ** H1 Cryostat X Position = " << m_CryoXPosition << "\n *** *** \n";
  std::cout << " ** H1 Table Y Position = " << m_TableYPosition << "\n *** *** \n";
}
