/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ActsFatrasWriteHandler.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_ACTSTOOLS_ACTSFATRASWRITEHANDLER_H
#define ISF_ACTSTOOLS_ACTSFATRASWRITEHANDLER_H

// GaudiKernel & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/WriteHandleKeyArray.h"


// ISF
#include "ISF_Event/ISFParticle.h"
// Acts
#include "Acts/Geometry/TrackingGeometry.hpp"
// ActsFatras
#include "ActsFatras/EventData/Particle.hpp"
#include "ActsFatras/EventData/Hit.hpp"
// ATLAS
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetSimEvent/SiHitCollection.h"


// STL
#include <utility>
#include <array>
#include <string>
  
// class IHitCreator;
    
/**
   @class ActsFatrasWriteHandler
    
    Write Handler for ISF and ActsFatras

    @author rui.wang@cern.ch
*/

class ActsFatrasWriteHandler: public AthAlgTool
{
public:
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
                  SiHitCollection& pixelSiHits, SiHitCollection& sctSiHits) const; // override final;

protected:
  const PixelID*                       m_pixIdHelper{};             //!< the Pixel ID helper
  const SCT_ID*                        m_sctIdHelper{};             //!< the SCT ID helper

  std::array<StringProperty,2> m_HitCollectionNames{{{this,"PixelCollectionName","PixelHits_ActsFatras"},{this,"SCTCollectionName","SCT_Hits_ActsFatras"}}};
  SG::WriteHandleKeyArray<SiHitCollection> m_HitCollectionKeys;

};

#endif 
