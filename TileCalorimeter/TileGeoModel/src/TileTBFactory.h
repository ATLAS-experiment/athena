/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TileTBFactory.h
 *
 * @brief Definition of TileTBFactory class
 *
 * @author Gia Khoriauli <Gia.Khoriauli@cern.ch>
 *
 */

#ifndef TileTBFactory_h
#define TileTBFactory_h 1

#include "CxxUtils/checker_macros.h"
#include "GeoModelKernel/GeoVDetectorFactory.h"
#include "TileDetDescr/TileDetDescrManager.h"
#include "TileGeoModel/TileSwitches.h"

class MsgStream;
class StoreGateSvc;


class ATLAS_NOT_THREAD_SAFE TileTBFactory : public GeoVDetectorFactory
//    ^ modifies cabling during create
{
 public:

  /** Constructor */
  TileTBFactory(StoreGateSvc *pDetStore, TileDetDescrManager *manager,
                const TileSwitches & switches, MsgStream *log);

  /** Destructor */
  ~TileTBFactory();

  /** Creation of Test Beam Tile geometry */
  virtual void create(GeoPhysVol *world) override;

  /** Access function to TileDetDescr geometry data */
  virtual const TileDetDescrManager * getDetectorManager() const override { return m_detectorManager; }

 private:

  /** Detector pointer to Store Gate service */
  StoreGateSvc              *m_detectorStore;

  /** Detector pointer to TileDetDescrManager */
  TileDetDescrManager       *m_detectorManager;

  /** Get message SVC */
  MsgStream                 *m_log;

  /** all switches */
  TileSwitches m_switches;

  /** Flag for activation verbose level for debugging */
  bool                       m_verbose;
};

#endif
