/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ActsFatrasWriteHandler.cxx
///////////////////////////////////////////////////////////////////

// class header
#include "ActsFatrasWriteHandler.h"
//InDet
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetSimEvent/SiHitCollection.h"

//Tracking ACTS
#include "ActsGeometry/ActsDetectorElement.h"
#include "TrkSurfaces/Surface.h"


ActsFatrasWriteHandler::ActsFatrasWriteHandler(const std::string& type, const std::string& name,
    const IInterface* parent)
  : AthAlgTool(type, name, parent){}

ActsFatrasWriteHandler::~ActsFatrasWriteHandler(){}

StatusCode ActsFatrasWriteHandler::initialize()
{
    ATH_MSG_INFO(name() << " initialize()" );
    // Get the Pixel Identifier-helper
    ATH_CHECK(detStore()->retrieve(m_pixIdHelper, "PixelID"));
    // Get the SCT Identifier-helper
    ATH_CHECK(detStore()->retrieve(m_sctIdHelper, "SCT_ID"));
    // setup handle keys
    for(auto const& HitCollectionName:m_HitCollectionNames){
        m_HitCollectionKeys.push_back(static_cast<std::string>(HitCollectionName));  
    }
    ATH_CHECK(m_HitCollectionKeys.initialize());

    ATH_MSG_INFO(name() << " initialize() successful" );
    return StatusCode::SUCCESS;
}

StatusCode ActsFatrasWriteHandler::WriteHits(std::vector<SiHitCollection>& HitCollections, const EventContext& ctx) const
{
    auto HitsWriteHandles = m_HitCollectionKeys.makeHandles(ctx);
    for(long unsigned int i=0;i<HitCollections.size();++i){
      ATH_MSG_DEBUG(name() << " WriteHits: adding "<<m_HitCollectionNames[i]<< " "<<HitCollections[i].size()<<" hits");
      ATH_CHECK(HitsWriteHandles[i].record(std::make_unique<SiHitCollection>(HitCollections[i])));
      ATH_MSG_DEBUG(name() << " WriteHits: added "<<m_HitCollectionNames[i]<< " "<<HitCollections[i].size()<<" hits");
    }
    return StatusCode::SUCCESS;
}

StatusCode ActsFatrasWriteHandler::finalize()
{
    return StatusCode::SUCCESS;
}

void ActsFatrasWriteHandler::createHits(const ISF::ISFParticle& isp,
                    std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry,
                    const std::vector<ActsFatras::Hit>& hits,
                    SiHitCollection& pixelSiHits, SiHitCollection& sctSiHits) const
{
    ATH_MSG_VERBOSE(name() << " particle " << isp << " with " << hits.size() << " hits");
    for (auto& hit:hits){
      double energyDeposit = hit.depositedEnergy();
      double time          = hit.time();

      // get the ACTS geo identifier
      auto hit_geoid = hit.geometryId();
      // get the ACTS surface
      try {
        auto acts_surface = trackingGeometry->findSurface(hit_geoid);
        const ActsDetectorElement* acts_de = dynamic_cast<const ActsDetectorElement*>(acts_surface->associatedDetectorElement());

        const Trk::Surface& hitSurface = acts_de->atlasSurface();
        ATH_MSG_VERBOSE(name() << " Surface position global atlas: " << hitSurface.center());
        // get the ATLAS identifier and hash identifier
        Identifier hitId         = hitSurface.associatedDetectorElementIdentifier(); 
        ATH_MSG_VERBOSE(name() << " hit Id Acts|atlas: "<< hit_geoid <<"|" << hitId);
        const Trk::TrkDetElementBase* detElementBase = hitSurface.associatedDetectorElement();
        const InDetDD::SiDetectorElement* hitSiDetElement = dynamic_cast<const InDetDD::SiDetectorElement*>((detElementBase));
        ATH_MSG_VERBOSE(name() << " hit position global Acts|atlas: " << hit.position() <<"|" << detElementBase->center());

        auto intersection = hitSurface.globalToLocal(hit.position());
        ATH_MSG_VERBOSE(name() << " hit isOnSurface: "<<hitSurface.isOnSurface(hit.position()));
        ATH_MSG_VERBOSE(name() << " intersection (atlas local) " << intersection.value().x() <<"," << intersection.value().y());
        double interX = intersection.value().x();
        double interY = intersection.value().y();

        double thickness = hitSiDetElement->thickness();
        ATH_MSG_VERBOSE(name() << " thickness (atlas)" << thickness);
        
        const Amg::Transform3D &hitTransform = hitSiDetElement->transformHit().inverse();
        // get the momentum direction into the local frame
        Amg::Vector3D particleDir = hit.direction();
        const Amg::Transform3D& sTransform = hitSurface.transform();
        Amg::Vector3D localDirection = sTransform.inverse().linear() * particleDir;   
        localDirection *= thickness/cos(localDirection.theta());
        // moving direction
        int movingDirection = localDirection.z() > 0. ? 1 : -1;
        // get he local distance of the intersection in x,y
        double distX = localDirection.x();
        double distY = localDirection.y();
        // local entries in x,y
        double localEntryX = interX-0.5*distX;
        double localEntryY = interY-0.5*distY;
        double localExitX  = interX+0.5*distX;
        double localExitY  = interY+0.5*distY;
        //!< @todo : fix edge effects 
        // transform into the hit frame
        Amg::Vector3D localEntry(hitTransform * (sTransform * Amg::Vector3D(localEntryX,localEntryY,-0.5*movingDirection*thickness)));
        Amg::Vector3D localExit(hitTransform * (sTransform * Amg::Vector3D(localExitX,localExitY,0.5*movingDirection*thickness)));
        const HepGeom::Point3D<double> localEntryHep( localEntry.x(), localEntry.y(), localEntry.z() );
        const HepGeom::Point3D<double> localExitHep( localExit.x(), localExit.y(), localExit.z() );

        auto isPixel=hitSiDetElement->isPixel();
        ATH_MSG_VERBOSE(name() << " localEntryHep|localExitHep (atlas)" << localEntryHep << "|" << localExitHep);
        ATH_MSG_VERBOSE(name() << " isPixel " << isPixel);
        ATH_MSG_VERBOSE(name() << " barrel_ec (atlas)" << (isPixel ? m_pixIdHelper->barrel_ec(hitId)  : m_sctIdHelper->barrel_ec(hitId)));
        ATH_MSG_VERBOSE(name() << " layer_disk (atlas)" << (isPixel ? m_pixIdHelper->layer_disk(hitId)  : m_sctIdHelper->layer_disk(hitId)));
        ATH_MSG_VERBOSE(name() << " eta_module (atlas)" << (isPixel ? m_pixIdHelper->eta_module(hitId)  : m_sctIdHelper->eta_module(hitId)));
        ATH_MSG_VERBOSE(name() << " phi_module (atlas)" << (isPixel ? m_pixIdHelper->phi_module(hitId)  : m_sctIdHelper->phi_module(hitId)));

        auto partLink = isp.getParticleLink();
        auto siHit = SiHit(localEntryHep,
                          localExitHep,
                          energyDeposit,
                          time,
                          *partLink,
                          isPixel ? 0 : 1,
                          isPixel ? m_pixIdHelper->barrel_ec(hitId)  : m_sctIdHelper->barrel_ec(hitId),
                          isPixel ? m_pixIdHelper->layer_disk(hitId) : m_sctIdHelper->layer_disk(hitId),
                          isPixel ? m_pixIdHelper->eta_module(hitId) : m_sctIdHelper->eta_module(hitId),
                          isPixel ? m_pixIdHelper->phi_module(hitId) : m_sctIdHelper->phi_module(hitId),
                          isPixel ? 0 : m_sctIdHelper->side(hitId)); 
        if (isPixel)
          pixelSiHits.push_back(siHit);
        else
          sctSiHits.push_back(siHit);
        ATH_MSG_VERBOSE(name() << " convert and store 1 hit, total" << pixelSiHits.size() << "Pixel | "<<sctSiHits.size()<< " SCT hits stored for particle" << isp);
      }
      catch (const std::exception& e){
        ATH_MSG_DEBUG(name() << "Can not find Acts Surface (" << e.what() << ")...Skip...");
      }
    }
}