/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ActsFatrasWriteHandler.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_ACTSTOOLS_ACTSFATRASWRITEHANDLER_H
#define ISF_ACTSTOOLS_ACTSFATRASWRITEHANDLER_H

// Must come before any Acts/Eigen header: enables the ATLAS Eigen plugin
#include "GeoPrimitives/GeoPrimitives.h"

// GaudiKernel & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/WriteHandleKey.h"


// ISF
#include "ISF_Event/ISFParticle.h"
// Acts
#include "Acts/Geometry/TrackingGeometry.hpp"
// ActsFatras
#include "ActsFatras/EventData/Particle.hpp"
#include "ActsFatras/EventData/Hit.hpp"
// ATLAS
#include "InDetSimEvent/SiHitCollection.h"


// STL
#include <array>
#include <memory>
#include <string>
#include <vector>

class PixelID;
class SCT_ID;
class PLR_ID;
class HGTD_ID;
namespace InDetDD {
  class SiDetectorManager;
}

/**
   @class ActsFatrasWriteHandler

    Write Handler for ISF and ActsFatras

    @author rui.wang@cern.ch
*/

class ActsFatrasWriteHandler: public AthAlgTool
{
public:
  /// Index of each hit collection, used for the property, the write key
  /// and the position in the vector passed to WriteHits()
  enum HitCollectionIndex : std::size_t { Pixel = 0, SCT = 1, HGTD = 2, PLR = 3, NCollections = 4 };

  /**Constructor */
  ActsFatrasWriteHandler(const std::string&,const std::string&,const IInterface*);

  /**Destructor*/
  ~ActsFatrasWriteHandler();

  // Athena algtool's Hooks
  virtual StatusCode  initialize() override;
  virtual StatusCode  finalize() override;

  // Hits read & write to SG
  StatusCode  WriteHits(std::vector<SiHitCollection>& HitCollections,const EventContext& ctx)  const;

  /** Loop over the hits and call the hit creator,
      provide the ActsFatras::Particle to create the hits */
  void createHits(const ISF::ISFParticle& isp,
                  std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry,
                  const std::vector<ActsFatras::Hit>& hits,
                  std::vector<SiHitCollection>& HitCollections) const;

protected:
  const PixelID*                       m_pixIdHelper{};             //!< the Pixel ID helper
  const SCT_ID*                        m_sctIdHelper{};             //!< the SCT ID helper
  const PLR_ID*                        m_plrIdHelper{};             //!< the PLR ID helper (optional)
  const HGTD_ID*                       m_hgtdIdHelper{};            //!< the HGTD ID helper (optional)

  /// Collection names; an empty name disables writing that collection
  std::array<StringProperty, NCollections> m_HitCollectionNames{{
      {this, "PixelCollectionName", "PixelHits_ActsFatras"},
      {this, "SCTCollectionName",   "SCT_Hits_ActsFatras"},
      {this, "HGTDCollectionName",  ""},
      {this, "PLRCollectionName",   ""}}};
  std::array<SG::WriteHandleKey<SiHitCollection>, NCollections> m_HitCollectionKeys;

  /// Strip detector manager, used to find the row of split side-1 modules via their side-0 partner
  Gaudi::Property<std::string> m_stripManagerName{this, "StripManagerName", "ITkStrip",
      "Strip detector manager (ITkStrip or SCT)"};
  const InDetDD::SiDetectorManager* m_stripManager{};
};

#endif