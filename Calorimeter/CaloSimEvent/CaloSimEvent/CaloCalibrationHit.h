/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// CaloCalibrationHit
// 26-Jan-2004 William Seligman

// This class defines a "calibration" hit associated with the Calo
// simulation.

#ifndef CaloSimEvent_CaloCalibrationHit_h
#define CaloSimEvent_CaloCalibrationHit_h

#include "Identifier/Identifier.h"
#include "TruthUtils/MagicNumbers.h"
#include "GeneratorObjects/HepMcParticleLink.h"

class CaloCalibrationHit 
/** @brief Class to store calorimeter calibration hit. <br>
    A calibration hit stores for active, inactive and dead material elements the energy loss according to the
    process (EM energy loss, non EM energy loss, Invisible energy and Escaped energy) */

{
 public:

  /** @brief Standard constructor using identifer and energy by type 
      @param[in] id  Cell identifier (calorimeter cell or dead material identifier)
      @param[in] energyEM  energy loss by EM processes
      @param[in] energyNonEM  visible energy loss by non EM processes (pion dEdx for instance)
      @param[in] energyInvisible  invisible energy loss (usually nuclear binding energy)
      @param[in] energyEscaped   energy which escaped from this cell because of production of neutrino (or escaping muon energy)
  */
  CaloCalibrationHit(Identifier id, 
		     double energyEM, 
		     double energyNonEM, 
		     double energyInvisible, 
		     double energyEscaped):
    m_ID(id), 
    m_energy0(energyEM), 
    m_energy1(energyNonEM), 
    m_energy2(energyInvisible), 
    m_energy3(energyEscaped)
  {}

  /** @brief Standard constructor using identifer, energy by type and primary particle ID
    @param[in] id  Cell identifier (calorimeter cell or dead material identifier)
    @param[in] energyEM  energy loss by EM processes
    @param[in] energyNonEM  visible energy loss by non EM processes (pion dEdx for instance)
    @param[in] energyInvisible  invisible energy loss (usually nuclear binding energy)
    @param[in] energyEscaped   energy which escaped from this cell because of production of neutrino (or escaping muon energy)
    @param[in] particleID barcode of primary particle which caused given hit
  */
  CaloCalibrationHit(Identifier id, 
                     double energyEM, 
                     double energyNonEM, 
                     double energyInvisible, 
                     double energyEscaped, 
                     int barcode,
                     int uniqueID = HepMC::INVALID_PARTICLE_ID):
  m_ID(id), 
  m_energy0(energyEM), 
  m_energy1(energyNonEM), 
  m_energy2(energyInvisible), 
  m_energy3(energyEscaped),
  m_barcode(barcode),
  m_uniqueID(uniqueID)
  {
    if (m_barcode == HepMC::UNDEFINED_ID) { m_uniqueID = HepMC::UNDEFINED_ID; } // No link to a truth particle
    else if (m_uniqueID == HepMC::INVALID_PARTICLE_ID && m_barcode != HepMC::INVALID_PARTICLE_ID) {
      m_partLink = std::make_unique<HepMcParticleLink>(m_barcode, 0, HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE); // FIXME is barcode-based
    }
  }

  /** Default constructor; should never be used, but provided for some
      persistency services. */
  CaloCalibrationHit():
    m_ID(Identifier())
  {}

  /** Copy constructor **/
  CaloCalibrationHit(const CaloCalibrationHit &cchSource)
    : m_ID (cchSource.m_ID),
      m_energy0 (cchSource.m_energy0),
      m_energy1 (cchSource.m_energy1),
      m_energy2 (cchSource.m_energy2),
      m_energy3 (cchSource.m_energy3),
      m_barcode (cchSource.m_barcode),
      m_uniqueID(cchSource.m_uniqueID)
  {
    if (m_barcode == HepMC::UNDEFINED_ID) { m_uniqueID = HepMC::UNDEFINED_ID; } // No link to a truth particle
    else if (m_uniqueID == HepMC::INVALID_PARTICLE_ID && m_barcode != HepMC::INVALID_PARTICLE_ID) {
      m_uniqueID = cchSource.particleUID(); // Try a look-up via HepMcParticleLink
    }
  }

  /** Assignment operator **/
  CaloCalibrationHit& operator= (const CaloCalibrationHit &cchSource)
  {
    if (this == &cchSource) return *this;
    m_ID = cchSource.m_ID;
    m_energy0 = cchSource.m_energy0;
    m_energy1 = cchSource.m_energy1;
    m_energy2 = cchSource.m_energy2;
    m_energy3 = cchSource.m_energy3;
    m_barcode = cchSource.m_barcode;
    m_uniqueID = cchSource.m_uniqueID;
    if (m_barcode == HepMC::UNDEFINED_ID) { m_uniqueID = HepMC::UNDEFINED_ID; } // No link to a truth particle
    else if (m_uniqueID == HepMC::INVALID_PARTICLE_ID && m_barcode != HepMC::INVALID_PARTICLE_ID) {
      m_uniqueID = cchSource.particleUID(); // Try a look-up via HepMcParticleLink
    }
    return *this;
  }

  /** Destructor */
  virtual ~CaloCalibrationHit() {}
  
  /** @return cell identifier of this hit */
  Identifier cellID()      const { return m_ID; }
  
  /** @return EM energy deposits.  Units are MeV. */
  double energyEM()        const { return  m_energy0; }

  /** @return NonEM energy deposits.  Units are MeV. */
  double energyNonEM()     const { return  m_energy1; }

  /** @return invisible energy.  Units are MeV. */
  double energyInvisible() const { return  m_energy2; }

  /** @return escaped energy.  Units are MeV. */
  double energyEscaped()   const { return  m_energy3; }

  /** @return total energy deposits.  Units are MeV. */
  double energyTotal()     const
  {
    return 
       m_energy0 + 
       m_energy1 + 
       m_energy2 + 
       m_energy3;
  }

  /** @return energy deposits by specifying input type.  Units are MeV. */
  double energy( unsigned int i ) const
  {
    switch (i)
      {
      case 0: return  m_energy0;
      case 1: return  m_energy1;
      case 2: return  m_energy2;
      case 3: return  m_energy3;
      default: return 0.;
      }
  }

  /** @return primary particle identifier (barcode) which caused this hit */
  int particleID()      const { return m_barcode; }

  /** @return primary particle identifier (barcode) which caused this hit (alias for helper functions) */
  int barcode()      const { return particleID(); }

  /** @return primary particle identifier (id) which caused this hit */
  int particleUID()      const { if (m_uniqueID == HepMC::INVALID_PARTICLE_ID && m_partLink) { return m_partLink->id(); } else  { return m_uniqueID;} }

  /** @return energy deposits by specifying input type, same as above method */
  double operator() (unsigned int i) const { return energy(i); }

  /** Calibration hits are ordered by values of their identifiers */
  bool Less(const CaloCalibrationHit* h) const 
  {
    if(m_ID != h->m_ID){
      return m_ID < h->m_ID; 
    }else{
      if (particleID() == HepMC::INVALID_PARTICLE_ID || h->particleUID() == HepMC::INVALID_PARTICLE_ID) {
        return m_barcode < h->m_barcode;
      }
      else {
        return particleUID() < h->particleUID();
      }
    }
  }

  /** Calibration hits are ordered by values of their identifiers */
  bool Equals(const CaloCalibrationHit& h) const {
    bool equal = (m_ID == h.m_ID);
    const bool validBarcode(particleID() != HepMC::INVALID_PARTICLE_ID && h.particleID() != HepMC::INVALID_PARTICLE_ID);
    const bool validUID(particleUID() != HepMC::INVALID_PARTICLE_ID && h.particleUID() != HepMC::INVALID_PARTICLE_ID);
    equal &= (validBarcode || validUID);
    if ( validBarcode ) {
      equal &= (m_barcode == h.m_barcode);
    }
    if ( validBarcode ) {
      equal &= (particleUID() == h.particleUID());
    }
    return  equal;
  };

    /** Calibration hits are ordered by values of their identifiers */
    bool Equals(const CaloCalibrationHit* h) const {
      return Equals (*h);
    }

  /** Method used for energy accumulation */
  void Add(const CaloCalibrationHit* h)
  {
    m_energy0 += h->m_energy0;
    m_energy1 += h->m_energy1;
    m_energy2 += h->m_energy2;
    m_energy3 += h->m_energy3;
  }

private:

  /** identifier of the cell in which this hit occured. */
  Identifier m_ID;

  /** energies (in MeV) deposited in this hit.  In order, they represent:
   * EM energy deposited
   * non-EM energy deposited
   * "invisible" energy deposited
   * escaped energy
   * Energies are accumulated in double precision and stored as floats */
  double m_energy0{0.};
  double m_energy1{0.};
  double m_energy2{0.};
  double m_energy3{0.};

  /** legacy barcode of Primary Particle which caused this hit */
  int m_barcode{HepMC::UNDEFINED_ID};
  /** identifier of Primary Particle which caused this hit */
  int m_uniqueID{HepMC::UNDEFINED_ID};
  std::unique_ptr<HepMcParticleLink> m_partLink{}; // nullptr unless object was produced by reading TrackRecord_p1.

};

#endif  // CaloSimEvent_CaloCalibrationHit_h

