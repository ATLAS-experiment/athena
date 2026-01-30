/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MaterialTrackRecorder.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include "StoreGate/WriteHandle.h"

#include "G4Event.hh"
#include "G4Run.hh"
#include "G4Step.hh"

namespace ActsTrk
{
    MaterialTrackRecorder::MaterialTrackRecorder(const Config& config):
        AthMessaging(Gaudi::svcLocator()->service< IMessageSvc >( "MessageSvc" ),"MaterialTrackRecorder"),
        m_cfg(config)
    {}

    void MaterialTrackRecorder::BeginOfEventAction(const G4Event*)
    {
        ATH_MSG_DEBUG(" BeginOfEventAction");

        // Get the event context for the write handle below
        const EventContext& ctx = Gaudi::Hive::currentContext();

        SG::WriteHandle<RecordedMaterialTrackCollection> materialTracks(m_cfg.materialTrackCollectionName, ctx);

        // Only record if it does NOT already exist
        if (!materialTracks.isPresent()) {
            auto coll = std::make_unique<RecordedMaterialTrackCollection>();
            if (materialTracks.record(std::move(coll)).isFailure()) {
                ATH_MSG_ERROR("Failed to record RecordedMaterialTrackCollection with key "
                              << m_cfg.materialTrackCollectionName);
                m_rmtCollection = nullptr;
                return;
          }
        } else {
            // Sanity check (should normally not happen unless someone else records it)
            ATH_MSG_DEBUG("Collection already present: " << m_cfg.materialTrackCollectionName);
        }

        m_rmtCollection = materialTracks.ptr();

        if (!m_rmtCollection) {
            ATH_MSG_ERROR("Recorded collection pointer is null right after record/present for key "
                          << m_cfg.materialTrackCollectionName);
        }

        ATH_MSG_DEBUG("ctx event=" << ctx.eventID().event_number()
                      << " slot=" << ctx.slot() << " key=" << m_cfg.materialTrackCollectionName);
        ATH_MSG_DEBUG("recorded? present=" << materialTracks.isPresent() << " valid=" << materialTracks.isValid());
    }

    void MaterialTrackRecorder::EndOfEventAction(const G4Event*)
    {
        ATH_MSG_DEBUG(" EndOfEventAction");
    }

    void MaterialTrackRecorder::BeginOfRunAction(const G4Run*)
    {
        ATH_MSG_DEBUG(" BeginOfRunAction");
        m_rmtCollection = nullptr;
    }

    void MaterialTrackRecorder::UserSteppingAction(const G4Step* step)
    {
        ATH_MSG_DEBUG(" UserSteppingAction");

        if (!m_rmtCollection) {
            ATH_MSG_ERROR("No per-event collection pointer cached (m_coll is null).");
            return;
        }

        // Get the material & check if it is present
        G4Material* material = step->GetPreStepPoint()->GetMaterial();
        if (material == nullptr) {
            return;
        }

        // First check for exclusion
        std::string materialName = material->GetName();
        for (const auto& emat : m_cfg.excludeMaterials) {
            if (emat == materialName) {
                ATH_MSG_DEBUG("Exclude step in material '" << materialName << "'.");
                return;
            }
        }

        ATH_MSG_DEBUG("Performing a step with step size = "
        << s_convertLength * step->GetStepLength());

        // Quantities valid for elemental materials and mixtures
        double X0 = s_convertLength * material->GetRadlen();
        double L0 = s_convertLength * material->GetNuclearInterLength();
        double rho = s_convertDensity * material->GetDensity();

        // Get{A,Z} is only meaningful for single-element materials (according to
        // the Geant4 docs). Need to compute average manually.
        const G4ElementVector* elements = material->GetElementVector();
        const G4double* fraction = material->GetFractionVector();
        std::size_t nElements = material->GetNumberOfElements();

        double Ar = 0.;
        double Z = 0.;
        if (nElements == 1) {
            Ar = material->GetA() / (CLHEP::gram / CLHEP::mole);
            Z = material->GetZ();
        } else {
            for (std::size_t i = 0; i < nElements; i++) {
                Ar += elements->at(i)->GetA() * fraction[i] / (CLHEP::gram / CLHEP::mole);
                Z += elements->at(i)->GetZ() * fraction[i];
            }
        }

        // Construct passed material slab for the step
        const auto slab = Acts::MaterialSlab(Acts::Material::fromMassDensity(X0, L0, Ar, Z, rho), s_convertLength * step->GetStepLength());

        // Create the RecordedMaterialSlab
        Acts::MaterialInteraction mInteraction;
        mInteraction.position = convertPosition(step->GetPreStepPoint()->GetPosition());
        mInteraction.direction = convertDirection(step->GetPreStepPoint()->GetMomentum()).normalized();
        mInteraction.materialSlab = slab;
        mInteraction.pathCorrection = (step->GetStepLength() / CLHEP::mm);

        G4Track* g4Track = step->GetTrack();

        Acts::RecordedMaterialTrack rmTrack;
        Acts::Vector3 vertex = convertPosition(g4Track->GetVertexPosition());
        Acts::Vector3 direction = convertDirection(g4Track->GetMomentumDirection());
        rmTrack.first = {vertex, direction};
        rmTrack.second.materialInteractions.push_back(mInteraction);
        m_rmtCollection->push_back(rmTrack);
    }
}

