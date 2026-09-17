/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_GEANT4TOOLS_Geant4TruthIncident_H
#define ISF_GEANT4TOOLS_Geant4TruthIncident_H

// ISF includes
#include "ISF_Event/ITruthIncident.h" //inheritance

//G4 includes
#include "G4ThreeVector.hh" //typedef

// HepMC includes
#include "AtlasHepMC/SimpleVector.h" //typedef FourVector
#include "AtlasHepMC/GenParticle_fwd.h" //typedef GenParticlePtr

#include "AtlasDetDescr/AtlasRegion.h" //enum

// std
#include <vector>


// forward declarations
class G4Step;
class G4Track;

namespace ISF {
  class ISFParticle;
}


namespace iGeant4 {

  /** @class Geant4TruthIncident

      ISF_Geant4 specific implementation of the ISF::ITruthIncident

      @author Andreas.Schaelicke@cern.ch
   */

  class Geant4TruthIncident : public ISF::ITruthIncident {
    public:
      Geant4TruthIncident( const G4Step*,
                           AtlasDetDescr::AtlasRegion geoID);
      virtual ~Geant4TruthIncident() {};

      /** Return HepMC position of the truth vertex */
      const HepMC::FourVector&  position() const override final;

      /** Return category of the physics process represented by the truth incident (eg hadronic, em, ..) */
      int                       physicsProcessCategory() const override final;
      /** Return specific physics process code of the truth incident (eg ionisation, bremsstrahlung, ..)*/
      int physicsProcessCode() const override final;

      /** Return p^2 of the parent particle */
      double                    parentP2() const override final;
      /** Return pT^2 of the parent particle */
      double                    parentPt2() const override final;
      /** Return Ekin of the parent particle */
      double                    parentEkin() const override final;
      /** Return the PDG Code of the parent particle */
      int                       parentPdgCode() const override final;
      // /** Return the barcode of the parent particle */
      // int  parentBarcode() override final; // TODO Remove this method
      /** Return the unique ID of the parent particle */
      int  parentUniqueID() override final;
      /** Return the status of the parent particle */
      int  parentStatus() override final;
      /** Return a boolean whether or not the parent particle survives the incident */
      bool                      parentSurvivesIncident() const override final;
      /** Return the parent particle after the TruthIncident vertex (and give
          it a new barcode - HEPMC2) */
#ifdef HEPMC3
      HepMC::GenParticlePtr     parentParticleAfterIncident() override final;
#else
      HepMC::GenParticlePtr     parentParticleAfterIncident(int newBC) override final;
#endif

      /** Return p of the i-th child particle */
      const G4ThreeVector       childP(unsigned short index) const;
      /** Return p^2 of the i-th child particle */
      double                    childP2(unsigned short index) const override final;
      /** Return pT^2 of the i-th child particle */
      double                    childPt2(unsigned short index) const override final;
      /** Return Ekin of the i-th child particle */
      double                    childEkin(unsigned short index) const override final;
      /** Return the PDG Code of the i-th child particle */
      int                       childPdgCode(unsigned short index) const override final;
#ifndef HEPMC3
      /** Return the barcode of the i-th child particle (if defined as part of the TruthIncident) otherwise return 0 */
      int  childBarcode(unsigned short index) const override final; // TODO Remove - only used in one place in TruthSvc
#endif

      /**  The interaction classifications are described as follows:
           STD_VTX: interaction of a particle without a pre-defined decay;
           QS_SURV_VTX: a particle with a pre-defined decay under-going a
           non-destructive interaction;
           QS_DEST_VTX: a particle with a pre-defined decay under-going a
           destructive interaction other than its pre-defined decay;
           QS_PREDEF_VTX: a particle under-going its pre-defined decay */
      ISF::InteractionClass_t interactionClassification() const override final;

      // only called once accepted

      /** Return the parent particle as a HepMC particle type */
      HepMC::GenParticlePtr      parentParticle() override final;
      /** Return the i-th child as a HepMC particle type and assign the given
          Barcode to the simulator particle */
      HepMC::GenParticlePtr   childParticle(unsigned short index,
                                            int bc) override final;
    private:
      Geant4TruthIncident();
      /** prepare the child particles */
      inline void prepareChildren();

      /** check if the given G4Track represents a particle that is alive in ISF or ISF-G4 */
      inline bool particleAlive(const G4Track *track) const;

#ifdef HEPMC3
       HepMC::GenParticlePtr convert(const G4Track *particle, const bool secondary) const;
#else
       HepMC::GenParticlePtr convert(const G4Track *particle, const int barcode, const bool secondary) const;
#endif

      bool                          m_positionSet;
      HepMC::FourVector             m_position;
      const G4Step*                 m_step{};

      std::vector<const G4Track*>   m_children;

      HepMC::GenParticlePtr         m_parentParticleAfterIncident{};
   };

}

#endif // ISF_GEANT4TOOLS_Geant4TruthIncident_H
