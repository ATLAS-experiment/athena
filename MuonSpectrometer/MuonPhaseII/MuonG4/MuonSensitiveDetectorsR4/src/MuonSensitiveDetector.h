/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSENSITIVEDETECTORSR4_MUONSENSITIVEDETECTOR_H
#define MUONSENSITIVEDETECTORSR4_MUONSENSITIVEDETECTOR_H

#include <GeoPrimitives/GeoPrimitives.h>
///

#include <GaudiKernel/EventContext.h>
#include <StoreGate/ReadHandleKey.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <xAODMuonSimHit/MuonSimHitContainer.h>
#include <xAODMuonSimHit/MuonSimHitAuxContainer.h>
#include <HitManagement/AthenaHitsVector.h>
#include <HitManagement/HitCollectionMap.h>
#include <AthenaBaseComps/AthMessaging.h>

#include <G4VSensitiveDetector.hh>

class AtlasG4EventUserInfo;
class G4Track;
class HepMcParticleLink;

/** @brief Basic MuonSensitiveDetector class from which all the other sensitive detectors inherit. The base implementation 
 *         provides the output container to store gate, creates the GeometryContext of interest and unifies the criteria whether
 *         a Step shall be processed or not. Finally, it keeps track whether a particle from a G4 step has already been recorded
 *         and updates then the last hit accordingly */
namespace MuonG4R4 {
    /** @brief Helper type which can be filled into the HitCollectionMap
     *         storing the event content in the event */
    using MuonSimHitsVec = AthHitVec::AuxStoreHitCollection<xAOD::MuonSimHitContainer,
                                                            xAOD::MuonSimHitAuxContainer>;

    class MuonSensitiveDetector : public G4VSensitiveDetector, public AthMessaging {
        public:
            /** @brief Constructor
             *  @param name: Name of the Sensitive detctor / AthMessaging module
             *  @param output_key: Key under which the sim hits are written into store gate
             *  @param trf_storeKey: Location of the DetctorAlignmentStore holding the transformations per event
             *  @param detMgr: Pointer to the run-4 detector manager */
            MuonSensitiveDetector(const std::string& name, 
                                  const std::string& output_key,
                                  const std::string& trf_storeKey,
                                  const MuonGMR4::MuonDetectorManager* detMgr);
            /** @brief default desructor  */
            ~MuonSensitiveDetector() = default;

            /** Create the output container at the beginning of the event */
            virtual void Initialize(G4HCofThisEvent* HCE) override final;

        private:
            /** @brief Key under which the output container is stored in the G4 event */
            std::string m_writeKey{};
            /** @brief Pointer to the MuonSimHit output container */
            xAOD::MuonSimHitContainer* m_outContainer{};
            /** @brief G4 event user information holding the current event store/context */
            AtlasG4EventUserInfo* m_g4UserEventInfo{};
            /**  ReadHandleKey to the DetectorAlignmentStore caching
              *  the relevant transformations needed in this event */
            SG::ReadHandleKey<ActsTrk::DetectorAlignStore> m_trfCacheKey;

            /** @brief Returns the event user info for the current G4 event, if available */
            AtlasG4EventUserInfo* eventInfo() const;

            /** @brief Returns the last snap shot matching an already generated HepMcParticleLink 
             *  @param gasGapId: Identifier of the gasGap to consider
             *  @param hitStep: Pointer to the current step
             *. @param particleLink the existing particle link for the G4Track */
            xAOD::MuonSimHit* lastSnapShot(const Identifier& gasGapId,
                                           const G4Step* hitStep,
                                           const HepMcParticleLink& particleLink);

        protected:
            /** @brief Checks whether the current step shall be processed at all. 
             *         I.e. the particle needs to be charged, there's a minimum velocity needed
             *         and the step length must not vanish
             *  @param step: G4 step to consider */
            bool processStep(const G4Step* step) const;

            /** @brief Returns the current event context recorded in the G4 event info */
            const EventContext& eventContext() const;

            /** @brief Returns the current geometry context in the event */
            ActsTrk::GeometryContext getGeoContext() const;

            /** @brief Generates a HepMcParticleLink for the Geant4 track using the current event store
                @param track: the track for which particle link should be created */
            HepMcParticleLink genParticleLink(const G4Track* track) const;

            /** @brief Returns the last snap shot of the traversing particle. The G4 track
             *         must have stepped through the same volume. Otherwise, a nullptr is returned
             *  @param gasGapId: Identifier of the gasGap to consider
             *  @param hitStep: Pointer to the current step */
            xAOD::MuonSimHit* lastSnapShot(const Identifier& gasGapId,
                                           const G4Step* hitStep);

            /** @brief Records the G4Step in the sim hit. Hits are usually 
             *         expressed at the center point of the step. Except for low-energy
             *         electrons where the endpoint of the random walk path is just extended
             *  @param hitId: Identifier of the gas gap or the tube in which the hit is recorded
             *  @param toGaGap: Transform from the ATLAS global coordinates into the 
             *                  sensitive volume 
             *  @param hitStep: Step from which the particle's information and the pre and
             *                  post step positions are fetched */
            xAOD::MuonSimHit* propagateAndSaveStrip(const Identifier& hitId,
                                                    const Amg::Transform3D& toGasGap,
                                                    const G4Step* hitStep);

            /** @brief Saves the current Step as a xAOD::MuonSimHit snapshot
             *  @param hitId: Identifier of the gasGap/ tube where the hit was deposited
             *  @param hitPos: Local position of the hit expressed w.r.t gas gap coordinate system
             *  @param hitDir: Local direction of the hit expressed w.r.t gas gap coordinate system
             *  @param globTime: Global time of the hit
             *  @param hitStep: Pointer to the step from which the hit information was derived. */
            xAOD::MuonSimHit* saveHit(const Identifier& hitId,
                                      const Amg::Vector3D& hitPos,
                                      const Amg::Vector3D& hitDir,
                                      const double globTime,
                                      const G4Step* hitStep);

            /** @brief Pointer to the underlying detector manager */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

            
    };
}

#endif
