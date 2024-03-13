/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSALIGNMENTCONDALG_H
#define ACTSGEOMETRY_ACTSALIGNMENTCONDALG_H

// ATHENA
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/CondHandleKeyArray.h"

// PACKAGE
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometrySvc.h"
#include "ActsGeometryInterfaces/RawGeomAlignStore.h"

class ActsAlignmentCondAlg : public AthReentrantAlgorithm {
public:
    ActsAlignmentCondAlg(const std::string &name, ISvcLocator *pSvcLocator);
    virtual ~ActsAlignmentCondAlg();

    StatusCode initialize() override;
    StatusCode execute(const EventContext &ctx) const override;

    // Switch off reentrancy to avoid condition clashes
    bool isReEntrant() const override final { return false; }

private:
    SG::ReadCondHandleKeyArray<ActsTrk::RawGeomAlignStore> m_alignStoreKeys{this, "AlignmentStores", {}, ""};

    SG::WriteCondHandleKey<ActsGeometryContext> m_wchk{this, "ActsAlignmentKey", "ActsAlignment", "cond handle key"};

    ServiceHandle<IActsTrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};

    Gaudi::Property<bool> m_loadTrkGeoSvc{this, "LoadTrackingGeoSvc", true, 
                                          "Toggle whether the tracking geometry svc shall be retrieved"};
};

#endif
