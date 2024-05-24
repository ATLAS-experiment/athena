/*
  Copyright (C) 2002-204 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_NSWPLOTTINGALG_H
#define MUONGEOMODELTESTR4_NSWPLOTTINGALG_H

#include <map>     //for map
#include <memory>  //for unique_ptr

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "StoreGate/ReadCondHandleKey.h"

class TGraph;
class TH1;

namespace MuonGMR4{
class NswGeoPlottingAlg : public AthHistogramAlgorithm {
 public:
  NswGeoPlottingAlg(const std::string& name, ISvcLocator* pSvcLocator);

  StatusCode initialize() override;
  StatusCode execute() override;
  unsigned int cardinality() const override final { return 1; }

 private:
  int layerId(const Identifier& id) const;

  StatusCode initMicroMega();

  // MuonDetectorManager from the conditions store
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                          "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

  SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

  const MuonDetectorManager* m_detMgr{nullptr};
  Gaudi::Property<std::string> m_outFile{this, "OutFile", "NSWGeoPlots.root"};
  /// Map showing the active areas of the NSW to show the passivation
  std::map<IdentifierHash, TH1*> m_nswActiveAreas{};
  bool m_alg_run{false};
};

}
#endif