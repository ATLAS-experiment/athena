/* Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration */

#ifndef EFLOWREC_PFTRACKCALOEXTENSIONTOOL_H
#define EFLOWREC_PFTRACKCALOEXTENSIONTOOL_H

#include "eflowTrackExtrapolatorBaseAlgTool.h"

#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "AthenaBaseComps/AthAlgTool.h"

static const InterfaceID IID_PFTrackCaloExtensionTool("PFTrackCaloExtensionTool", 1, 0);

/**
 Inherits from eflowTrackExtrapolatorBaseAlgTool and AthAlgTool. Uses ACTS to extrapolate tracks to the calorimeter, and creates an eflowTrackCaloPoints object.
*/
class PFTrackCaloExtensionTool: virtual public eflowTrackExtrapolatorBaseAlgTool, public AthAlgTool {

public:
  PFTrackCaloExtensionTool(const std::string& type, const std::string& name,
                             const IInterface* parent);
  ~PFTrackCaloExtensionTool() {};

  static const InterfaceID& interfaceID();

  virtual StatusCode initialize() override;
  virtual std::unique_ptr<eflowTrackCaloPoints> execute(const EventContext& ctx, const xAOD::TrackParticle* track) const override;
  virtual StatusCode finalize() override;

private:

     ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool {
     this,
     "ExtrapolationTool",
     "ExtrapolationTool",
     "Tool to run propagation in an ACTS tracking geometry"
    };

    ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};

    std::map<std::string, Acts::GeometryIdentifier> m_caloNameGeoIDMap;

};
#endif