/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_DETFACTORY_H
#define ZDC_DETFACTORY_H

#include "GeoModelKernel/GeoVDetectorFactory.h"
#include "ZDC_DetManager.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "ZdcIdentifier/ZdcID.h"
#include "GeoModelInterfaces/StoredMaterialManager.h"

class StoreGateSvc;

class ZDC_DetFactory : public GeoVDetectorFactory,
                       public AthMessaging
{

public:
  ZDC_DetFactory(StoreGateSvc *);
  ~ZDC_DetFactory();

  virtual void create(GeoPhysVol *world) override;
  virtual const ZDC_DetManager *getDetectorManager() const override;
  void buildMaterials(StoredMaterialManager *materialManager);

  void initializePbPb2015();
  void initializePbPb2023();

private:

  ZDC_DetManager *m_detectorManager;
  StoreGateSvc *m_detectorStore;
  const ZdcID *m_zdcID;
  bool m_RPDs_On; //Flag for both RPD modules
  bool m_BRANs_On; //Flag for both BRAN modules
  std::vector< std::vector< bool > > m_zdcOn;
  std::vector< std::vector< float > > m_zdcPos; //Positions of the ZDC modules
  std::vector< std::vector< std::pair<int,int> > > m_zdcPixelStart_Stop; //Start and stop layers of the pixels for a given ZDC module
  std::vector< float > m_rpdPos; //Positions of the RPD modules
  std::vector< float > m_branPos; //Positions of the BRAN modules
};




#endif
