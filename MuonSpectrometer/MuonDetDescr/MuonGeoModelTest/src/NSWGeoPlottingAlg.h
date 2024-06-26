/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GEOMODELCHECK_NSWPLOTTINGALG_H
#define GEOMODELCHECK_NSWPLOTTINGALG_H
#include <map>     //for map
#include <memory>  //for unique_ptr

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadCondHandleKey.h"

class TGraph;
class TH1;

class NSWGeoPlottingAlg : public AthHistogramAlgorithm {
 public:
  NSWGeoPlottingAlg(const std::string& name, ISvcLocator* pSvcLocator);

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;
  unsigned int cardinality() const override final { return 1; }

 private:
  int layerId(const Identifier& id) const;

  StatusCode initMicroMega();
  StatusCode initSTgcs();

  // MuonDetectorManager from the conditions store
  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{
      this, "DetectorManagerKey", "MuonDetectorManager",
      "Key of input MuonDetectorManager condition data"};

  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
      this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  Gaudi::Property<std::string> m_outFile{this, "OutFile", "NSWGeoPlots.root"};
  /// Map containing each PCB of the NSW seperately
  std::map<Identifier, std::unique_ptr<TGraph>> m_nswPads{};
  /// Map showing the edges of the 16 layers of the NSW
  std::map<int, std::unique_ptr<TGraph>> m_nswLayers{};
  /// Map showing the active areas of the NSW to show the passivation
  std::map<int, std::unique_ptr<TH1>> m_nswActiveAreas{};
  bool m_alg_run{false};
};

#endif