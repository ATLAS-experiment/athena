/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ActsFatrasWriteHandler.cxx
///////////////////////////////////////////////////////////////////

// class header
#include "ActsFatrasWriteHandler.h"
//InDet
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/PLR_ID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetReadoutGeometry/SiDetectorManager.h"
#include "InDetSimEvent/SiHitCollection.h"
#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"
//HGTD
#include "HGTD_Identifier/HGTD_ID.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
// Common base of all silicon elements (Pixel, SCT, ITk, PLR, HGTD)
#include "ReadoutGeometryBase/SolidStateDetectorElementBase.h"

//Tracking ACTS
#include "ActsGeometry/ActsDetectorElement.h"
#include "TrkSurfaces/Surface.h"

#include <cmath>

namespace {
  /// Silicon detactors 
  constexpr int s_partPixel = 0;
  constexpr int s_partSCT   = 1;
  constexpr int s_partHGTD  = 2;
}

ActsFatrasWriteHandler::ActsFatrasWriteHandler(const std::string& type, const std::string& name,
    const IInterface* parent)
  : AthAlgTool(type, name, parent){}

ActsFatrasWriteHandler::~ActsFatrasWriteHandler(){}

StatusCode ActsFatrasWriteHandler::initialize()
{
    // Pixel and SCT (also used for ITkPixel / ITkStrip)
    ATH_CHECK(detStore()->retrieve(m_pixIdHelper, "PixelID"));
    ATH_CHECK(detStore()->retrieve(m_sctIdHelper, "SCT_ID"));
    // PLR and HGTD are optional: required only if their collection is written
    if (!m_HitCollectionNames[PLR].value().empty()) {
        ATH_CHECK(detStore()->retrieve(m_plrIdHelper, "PLR_ID"));
    }
    if (!m_HitCollectionNames[HGTD].value().empty()) {
        ATH_CHECK(detStore()->retrieve(m_hgtdIdHelper, "HGTD_ID"));
    }
    // setup handle keys; an empty name disables that collection
    for (std::size_t i = 0; i < NCollections; ++i) {
        const std::string& collName = m_HitCollectionNames[i].value();
        m_HitCollectionKeys[i] = collName;
        ATH_CHECK(m_HitCollectionKeys[i].initialize(!collName.empty()));
        ATH_MSG_INFO(name() << " hit collection [" << i << "]: "
                     << (collName.empty() ? std::string("(disabled)") : collName));
    }

    // ITkStrip detector manager: used for the row of split side-1 modules (via the
    // side-0 partner)
    if (!m_HitCollectionNames[SCT].value().empty()) {
        if (detStore()->retrieve(m_stripManager, m_stripManagerName.value()).isFailure() || !m_stripManager) {
            ATH_MSG_WARNING(name() << " no SiDetectorManager '" << m_stripManagerName.value()
                            << "' in the detector store: no row lookup via the other side");
            m_stripManager = nullptr;
        }
    }

    return StatusCode::SUCCESS;
}

StatusCode ActsFatrasWriteHandler::WriteHits(std::vector<SiHitCollection>& HitCollections, const EventContext& ctx) const
{
    for (std::size_t i = 0; i < HitCollections.size() && i < NCollections; ++i) {
      if (m_HitCollectionKeys[i].empty()) {
        if (!HitCollections[i].empty()) {
          ATH_MSG_WARNING(name() << " " << HitCollections[i].size() << " hits in collection [" << i
                          << "] are dropped because its collection name is not set");
        }
        continue;
      }
      SG::WriteHandle<SiHitCollection> handle(m_HitCollectionKeys[i], ctx);
      ATH_MSG_DEBUG(name() << " WriteHits: adding " << m_HitCollectionNames[i].value() << " "
                    << HitCollections[i].size() << " hits");
      ATH_CHECK(handle.record(std::make_unique<SiHitCollection>(HitCollections[i])));
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
                    std::vector<SiHitCollection>& HitCollections) const
{
    ATH_MSG_VERBOSE(name() << " particle " << isp << " with " << hits.size() << " hits");
    for (auto& hit:hits){
      double energyDeposit = ActsTrk::energyToAthena(hit.depositedEnergy());
      double time          = ActsTrk::timeToAthena(hit.time());

      // get the ACTS geo identifier
      auto hit_geoid = hit.geometryId();
      // get the ACTS surface
      try {
        auto acts_surface = trackingGeometry->findSurface(hit_geoid);
        const auto* acts_de = getActsDetectorElement(acts_surface);
        if (!acts_de) continue;

        // Only silicon elements (Pixel, SCT, ITk, PLR, HGTD) produce SiHits.
        // Check before atlasSurface(), which throws for TRT elements.
        const auto* solidDE = dynamic_cast<const InDetDD::SolidStateDetectorElementBase*>(
            acts_de->upstreamDetectorElement());
        if (!solidDE) continue;
        const auto* hitSiDetElement   = dynamic_cast<const InDetDD::SiDetectorElement*>(solidDE);
        const auto* hitHgtdDetElement = dynamic_cast<const InDetDD::HGTD_DetectorElement*>(solidDE);
        if (!hitSiDetElement && !hitHgtdDetElement) continue;

        const Trk::Surface& hitSurface = acts_de->atlasSurface();
        ATH_MSG_VERBOSE(name() << " Surface position global atlas: " << hitSurface.center());
        // get the ATLAS identifier and hash identifier
        Identifier hitId           = hitSurface.associatedDetectorElementIdentifier();
        ATH_MSG_VERBOSE(name() << " hit Id Acts|atlas: "<< hit_geoid <<"|" << hitId);
        ATH_MSG_VERBOSE(name() << " hit position global Acts|atlas: " << hit.position() <<"|" << solidDE->center());

        auto intersection = hitSurface.globalToLocal(hit.position());
        ATH_MSG_VERBOSE(name() << " hit isOnSurface: "<<hitSurface.isOnSurface(hit.position()));
        ATH_MSG_VERBOSE(name() << " intersection (atlas local) " << intersection.value().x() <<"," << intersection.value().y());
        double interX = intersection.value().x();
        double interY = intersection.value().y();

        double thickness = solidDE->thickness();

        const Amg::Transform3D hitTransform = solidDE->transformHit().inverse();
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

        auto truth_barcode = isp.barcode();

        // HGTD
        if (hitHgtdDetElement) {
          if (m_HitCollectionKeys[HGTD].empty() || !m_hgtdIdHelper) continue;
          // Same field order the HGTD digitization uses to rebuild the wafer id
          int endcap     = m_hgtdIdHelper->endcap(hitId);
          int layer      = m_hgtdIdHelper->layer(hitId);
          int phi_module = m_hgtdIdHelper->phi_module(hitId);
          int eta_module = m_hgtdIdHelper->eta_module(hitId);
          HitCollections[HGTD].push_back(SiHit(localEntryHep, localExitHep, energyDeposit, time,
                                               truth_barcode, s_partHGTD,
                                               endcap, layer, eta_module, phi_module, 0));
          continue;
        }

        // PLR: pixel-type SiDetectorElement, PLR identifier (barrel_ec = +-4)
        if (hitSiDetElement->isPLR()) {
          if (m_HitCollectionKeys[PLR].empty() || !m_plrIdHelper) continue;
          int barrel_ec  = m_plrIdHelper->barrel_ec(hitId);
          int layer_disk = m_plrIdHelper->layer_disk(hitId);
          int eta_module = m_plrIdHelper->eta_module(hitId);
          int phi_module = m_plrIdHelper->phi_module(hitId);
          HitCollections[PLR].push_back(SiHit(localEntryHep, localExitHep, energyDeposit, time,
                                              truth_barcode, s_partPixel,
                                              barrel_ec, layer_disk, eta_module, phi_module, 0));
          continue;
        }

        // Pixel / SCT (incl. ITkPixel / ITkStrip)
        const bool isPixel = hitSiDetElement->isPixel();
        const bool isSCT   = hitSiDetElement->isSCT();
        if (!isPixel && !isSCT) continue;
        const int hit_detector_region = isPixel ? s_partPixel : s_partSCT;

        // ITk strips: undo the row split. The digitizer builds the child ids as
        // hitEta + rowKey, so the hit must carry the eta of the row-0 wafer.
        int rowOffset = 0;
        if (isSCT) {
          const auto* design = dynamic_cast<const InDetDD::SCT_ModuleSideDesign*>(&hitSiDetElement->design());
          if (!design) {
            ATH_MSG_WARNING(name() << " SCT element " << hitId << " has no SCT_ModuleSideDesign, skipping hit");
            continue;
          }
          // Same logic as ITk::StripDigitizationTool::digitizeElement
          const auto* mother = design->getMother();
          if (mother) {
            bool found = false;
            for (const auto& [row, child] : mother->getChildren()) {
              if (child == design) { rowOffset = row; found = true; break; }
            }
            // ITk strip barrel side 1: the element shares the mother of side 0, but the
            // mother's children map only holds the side-0 row designs. The digitizer only
            // uses the row keys, so take the row of the side-0 partner (same module, same
            // eta index, other side).
            if (!found && m_stripManager) {
              const Identifier partnerId = m_sctIdHelper->wafer_id(
                  m_sctIdHelper->barrel_ec(hitId), m_sctIdHelper->layer_disk(hitId),
                  m_sctIdHelper->phi_module(hitId), m_sctIdHelper->eta_module(hitId),
                  1 - m_sctIdHelper->side(hitId));
              const InDetDD::SiDetectorElement* partner = m_stripManager->getDetectorElement(partnerId);
              const auto* partnerDesign = partner
                  ? dynamic_cast<const InDetDD::SCT_ModuleSideDesign*>(&partner->design()) : nullptr;
              if (partnerDesign && partnerDesign->getMother() == mother) {
                for (const auto& [row, child] : mother->getChildren()) {
                  if (child == partnerDesign) { rowOffset = row; found = true; break; }
                }
              }
            }
            if (!found) {
              ATH_MSG_WARNING(name() << " ITkStrip element " << hitId
                              << " has a mother design but is not among its children, skipping hit");
              continue;
            }
          }
          // else: legacy SCT, or ITkStrip without row splitting -> rowOffset stays 0.
        }

        int barrel_ec  = isPixel ? m_pixIdHelper->barrel_ec(hitId)  : m_sctIdHelper->barrel_ec(hitId);
        int layer_disk = isPixel ? m_pixIdHelper->layer_disk(hitId) : m_sctIdHelper->layer_disk(hitId);
        int eta_module = (isPixel ? m_pixIdHelper->eta_module(hitId) : m_sctIdHelper->eta_module(hitId)) - rowOffset;
        int phi_module = isPixel ? m_pixIdHelper->phi_module(hitId) : m_sctIdHelper->phi_module(hitId);
        int side       = isPixel ? 0 : m_sctIdHelper->side(hitId);

        auto siHit = SiHit(localEntryHep, localExitHep, energyDeposit, time,
                           truth_barcode,
                           hit_detector_region, barrel_ec, layer_disk, eta_module, phi_module, side);
        HitCollections[isPixel ? Pixel : SCT].push_back(siHit);
      }
      catch (const std::exception& e){
        ATH_MSG_DEBUG(name() << " Can not find Acts Surface (" << e.what() << ")...Skip...");
      }
    }
}