/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_ACTSTOOL_H
#define TRIGINDETPATTRECOTOOLS_ACTSTOOL_H

#include "ActsToolInterfaces/ISeedingTool.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "BeamSpotConditionsData/BeamSpotData.h"

#include "SeedingToolBase.h"

class Gbts2ActsSeedingTool: public SeedingToolBase<const xAOD::SpacePoint*>, public ActsTrk::ISeedingTool{
    public:

    // standard AlgTool methods
    Gbts2ActsSeedingTool(const std::string&,const std::string&,const IInterface*);
    virtual ~Gbts2ActsSeedingTool(){};

    // standard Athena methods
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    // Interface
    virtual StatusCode createSeeds(const EventContext& ctx,
                  const Acts::SpacePointContainer<ActsTrk::SpacePointCollector, Acts::detail::RefHolder>& spContainer,
                  const Acts::Vector3& beamSpotPos,
                  const Acts::Vector3& bField,
                  ActsTrk::SeedContainer& seedContainer ) const override;

    protected:
    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey{this, "PixelDetectorElements", "ITkPixelDetectorElementCollection", "Key of input SiDetectorElementCollection for Pixel"};
    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_stripDetEleCollKey{this, "StripDetectorElements", "ITkStripDetectorElementCollection", "Key of input SiDetectorElementCollection for Strip"};
    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };
};
#endif
