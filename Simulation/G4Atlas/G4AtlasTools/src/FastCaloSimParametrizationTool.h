/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASTOOLS_FASTCALOSIMPARAMETRIZATIONTOOL_H
#define G4ATLASTOOLS_FASTCALOSIMPARAMETRIZATIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IFastCaloSimParametrizationTool.h"
#include "Gaudi/Property.h"

#include "FastCaloSim/Core/TFCSExtrapolationState.h"
#include "FastCaloSim/Core/TFCSParametrizationBase.h"
#include "FastCaloSim/Core/TFCSSimulationState.h"
#include "FastCaloSim/Core/TFCSTruthState.h"
#include "FastCaloSim/Extrapolation/FastCaloSimCaloExtrapolation.h"
#include "FastCaloSim/Transport/G4CaloTransportTool.h"

#include <memory>

class CaloGeo;

/// Owns the shared FastCaloSim geometry and parametrization model.
class FastCaloSimParametrizationTool
    : public extends<AthAlgTool, IFastCaloSimParametrizationTool> {

 public:
  FastCaloSimParametrizationTool(const std::string &, const std::string &,
                                 const IInterface *);
  // Defined out of line because CaloGeo is forward-declared here.
  ~FastCaloSimParametrizationTool() override;

  StatusCode initialize() override final;
  StatusCode finalize() override final;

  StatusCode initializeTransportGeometry() override final;
  StatusCode initializeTransportPropagator() override final;

  std::vector<G4FieldTrack> transport(
      const G4Track &G4InputTrack) override final;
  void extrapolate(
      TFCSExtrapolationState &result, const TFCSTruthState *truth,
      const std::vector<G4FieldTrack> &caloSteps) override final;
  FCSReturnCode simulate(
      TFCSSimulationState &simulstate, const TFCSTruthState *truth,
      const TFCSExtrapolationState *extrapol) override final;

 private:
  Gaudi::Property<std::string> m_paramsFilename{
      this, "ParamsInputFilename", "",
      "Filename of the input parametrizations file"};
  Gaudi::Property<std::string> m_paramsObject{
      this, "ParamsInputObject", "SelPDGID",
      "Name of the parametrization object in the input file"};
  Gaudi::Property<std::string> m_geoInputFolder{
      this, "CaloGeoInputFolder", "FastCaloSim/MC23/GeoFiles",
      "Folder holding the FastCaloSim calorimeter geometry files"};
  Gaudi::Property<std::string> m_geoTag{
      this, "CaloGeoTag", "",
      "Geometry tag prefix of the CaloCells geometry file"};
  Gaudi::Property<std::string> m_simplifiedGeoPath{
      this, "SimplifiedGeoPath", "FastCaloSim/MC23/GeoFiles/v02/simplified_geo.gdml",
      "Calib-area path of the simplified transport-geometry GDML file"};

  std::unique_ptr<CaloGeo> m_caloGeo;
  std::unique_ptr<TFCSParametrizationBase> m_param;
  G4CaloTransportTool m_caloTransportTool;
  FastCaloSimCaloExtrapolation m_caloExtrapolationTool;

};  // class FastCaloSimParametrizationTool

#endif  // G4ATLASTOOLS_FASTCALOSIMPARAMETRIZATIONTOOL_H
