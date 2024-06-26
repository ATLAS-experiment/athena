/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GEOMODELCHECK_STGCPADPLOTTINGALG_H
#define GEOMODELCHECK_STGCPADPLOTTINGALG_H
#include <map>     //for map
#include <memory>  //for unique_ptr

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadCondHandleKey.h"

class TGraph;
class TH2;
/**
 *  Simple algorithm to plot the sTGC pad positions
*/

class sTgcPadPlottingAlg : public AthHistogramAlgorithm {
 public:
  sTgcPadPlottingAlg(const std::string& name, ISvcLocator* pSvcLocator);

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;
  unsigned int cardinality() const override final { return 1; }

 private:
  
  int layerId(const Identifier& id) const;
  std::string padName(const Identifier& padId) const;
  StatusCode initSTgcs();

  // MuonDetectorManager from the conditions store
  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{
      this, "DetectorManagerKey", "MuonDetectorManager",
      "Key of input MuonDetectorManager condition data"};

  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
      this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  Gaudi::Property<std::string> m_outFile{this, "OutFile", "sTgcPadPlots.root"};
  
  /// Map containing each PCB of the NSW seperately
  std::map<Identifier, std::unique_ptr<TGraph>> m_nswPads{};
  std::map<Identifier, std::unique_ptr<TH2>> m_nswPadDist{};
  bool m_alg_run{false};
};

#endif