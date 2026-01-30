/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_MATERIALTRACKRECORDER_H
#define ACTSGEOMETRY_MATERIALTRACKRECORDER_H

#include "AthenaBaseComps/AthMessaging.h"
#include "ActsMaterial/RecordedMaterialTrackCollection.h"

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/Units.hpp"
#include "CLHEP/Units/SystemOfUnits.h"

#include "G4UserEventAction.hh"
#include "G4UserRunAction.hh"
#include "G4UserSteppingAction.hh"
#include "G4ThreeVector.hh"

class G4Event;
class G4Run;
class G4Step;


namespace ActsTrk
{
    /// @class MaterialTrackRecorder
    ///
    /// @brief Collects the G4 steps and writes out RecordedMaterialTrackCollection to a store gate
    ///
    /// It writes out a MaterialTrack which is usually generated from
    /// MaterialTrackRecorder G4UA

    class MaterialTrackRecorder: public AthMessaging, public G4UserEventAction,
                                 public G4UserRunAction, public G4UserSteppingAction
    {
        public:
            struct Config {
                /// Output collection name for recorded material tracks
                std::string materialTrackCollectionName = "MaterialTracks";
                std::vector<std::string> excludeMaterials = {"Air", "Vacuum"};

            };

            MaterialTrackRecorder(const Config& config);
            virtual void BeginOfEventAction(const G4Event*) override;
            virtual void EndOfEventAction(const G4Event*) override;
            virtual void BeginOfRunAction(const G4Run*) override;
            virtual void UserSteppingAction(const G4Step*) override;

        private:
            /// Pointer to the collection, non-owning
            RecordedMaterialTrackCollection* m_rmtCollection{nullptr};
            /// Conversion constants
            constexpr static double s_convertLength = Acts::UnitConstants::mm / CLHEP::mm;
            constexpr static double s_convertDensity =
                (Acts::UnitConstants::g / Acts::UnitConstants::mm3) / (CLHEP::gram / CLHEP::mm3);

            /// The config class
            Config m_cfg;

            /// Useful fuction for position coversion
            Acts::Vector3 convertPosition(const G4ThreeVector& g4vec) {
                return Acts::Vector3(g4vec[0] * s_convertLength, g4vec[1] * s_convertLength,
                                    g4vec[2] * s_convertLength);
            }
            /// Useful fuction for direction coversion
            Acts::Vector3 convertDirection(const G4ThreeVector& g4vec) {
                return Acts::Vector3{g4vec[0], g4vec[1], g4vec[2]};
            }

    }; // class MaterialTrackRecorder

} // namespace ActsTrk

#endif

