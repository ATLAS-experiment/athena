/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TileDetectorTool_H
#define TileDetectorTool_H

#include "GeoModelUtilities/GeoModelTool.h"
#include "TileGeoModel/TileSwitches.h"
#include "Gaudi/Property.h"

#include <string>
#include <vector>

class StoreGateSvc;
class TileDetDescrManager;

class TileDetectorTool final : public GeoModelTool
{
 public:

  TileDetectorTool(const std::string& type,
                   const std::string& name,
                   const IInterface* parent);

  virtual ~TileDetectorTool() override final;

  virtual StatusCode create() override final;
  virtual StatusCode clear() override final;

  bool isTestBeam() const { return m_switches.testBeam; }
  bool isAddPlates() const { return m_switches.addPlatesToCell; }
  int uShape() const { return m_switches.uShape; }

 private:

  StatusCode initIds();
  StatusCode createElements();
  void setSwitch(int & param, int value, const char * name);
  // Copy property values into m_switches
  void fillSwitches();

  TileSwitches m_switches{false, true};

  // Properties copied into m_switches by fillSwitches()
  Gaudi::Property<bool> m_testBeam{this, "TestBeam", false};
  // AddPlatesToCell is not a property, to make user aware that the value is taken from DB rather from JO
  Gaudi::Property<int> m_uShape{this, "Ushape", -1};
  Gaudi::Property<int> m_glue{this, "Glue", -1};
  Gaudi::Property<int> m_pvt{this, "PVT", -1};
  Gaudi::Property<int> m_steel{this, "Steel", -1};
  Gaudi::Property<int> m_csTube{this, "CsTube", -1};
  Gaudi::Property<int> m_crackOption{this, "CrackOption", 0};

  Gaudi::Property<std::vector<std::string>> m_volumes{this, "TopVolumes", {}, "List of top-level volumes where crack scintillators will be placed"};

  bool m_not_locked{true};
  Gaudi::Property<bool> m_useNewFactory{this, "UseNewFactory", true};
  Gaudi::Property<std::string> m_geometryConfig{this, "GeometryConfig", "FULL"}; // FULL, SIMU, RECO --- A note: It's set to FULL by default in the constructor, and it seems that SIMU is never used in Athena code; also, the code that calls the TileAtlasFactory only checks if it's FULL or not:  m_geometryConfig=='FULL' (RMB)
  TileDetDescrManager* m_manager{nullptr};
};

#endif
