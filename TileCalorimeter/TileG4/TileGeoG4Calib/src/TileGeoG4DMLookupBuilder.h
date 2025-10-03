/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class building look-up tables for TileGeoG4CalibSD
//
// Author: Gia Khoriauli <gia.khoriauli@cern.ch>
//
// May, 2005
//
//************************************************************

#ifndef TILEGEOG4CALIB_TILEGEOG4DMLOOKUPBUILDER_H
#define TILEGEOG4CALIB_TILEGEOG4DMLOOKUPBUILDER_H

#include <map>
#include <memory>

#include "GaudiKernel/ServiceHandle.h"
#include "TileDetDescr/TileDddbManager.h"
#include "TileDetDescr/TileDetDescrManager.h"
#include "TileCalibDddbManager.h"

class TileGeoG4CalibSection;
class TileGeoG4LookupBuilder;
class IRDBAccessSvc;
class IGeoModelSvc;
class StoreGateSvc;

class TileGeoG4DMLookupBuilder {
public:
  TileGeoG4DMLookupBuilder(TileGeoG4LookupBuilder* tile_lookup_builder, ServiceHandle<IRDBAccessSvc> &access,
                           ServiceHandle<IGeoModelSvc> &geo_svc, const ServiceHandle<StoreGateSvc> &pDetStore,
                           const int verboseLevel);

  void BuildLookup(bool test_beam = false, int plateToCell = -1);
  void ResetCells();
  TileGeoG4CalibSection* GetSection(TileCalibDddbManager::TileCalibSections key) const;
  bool GetPlateToCell();

  //Geometry constans fo DH calculator
  double rBMin, rBMax;
  double zBarrMaxPos, zBarrMaxNeg;
  double dzBarrMod, dzExtBarrMod;
  double zLegngthITC;
  double dzBarrPeriod, dzExtBarrPeriod;
  double rGirdMin;
  double dRFront;
  double dZEnd, dZEndSh;

  double rP1Min, rP2Min, rGapMax, rGapMin, rCrMax, rCrMin;

private:

  TileGeoG4DMLookupBuilder(const TileGeoG4DMLookupBuilder&);
  TileGeoG4DMLookupBuilder& operator=(const TileGeoG4DMLookupBuilder&);

  typedef std::map<TileCalibDddbManager::TileCalibSections, TileGeoG4CalibSection*,
                   std::less<TileCalibDddbManager::TileCalibSections> > TileGeoG4CalibSectionMap;

  void CreateGeoG4CalibSections(bool is_ctb, int plateToCell);

  std::unique_ptr<TileCalibDddbManager> m_dbManager;
  TileGeoG4LookupBuilder* m_lookup_builder;
  std::unique_ptr<TileGeoG4CalibSectionMap> m_sectionMap;

  const TileDetDescrManager* m_theManager;
  TileDddbManager* m_tdbManager;

  int m_verboseLevel;

  bool m_plateToCell;
};

#endif // TILEGEOG4CALIB_TILEGEOG4DMLOOKUPBUILDER_H
