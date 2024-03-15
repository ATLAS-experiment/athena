///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATOROBJECTSTPCNV_HEPMCPARTICLELINK_P3_H
#define GENERATOROBJECTSTPCNV_HEPMCPARTICLELINK_P3_H

// STL includes
#include <string>

// Forward declaration
class HepMcParticleLinkCnv_p3;

class HepMcParticleLink_p3
{
  // Make HepMcParticleLinkCnv_p3 our friend
  friend class HepMcParticleLinkCnv_p3;

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
public:

  /** Default constructor:
   */
  HepMcParticleLink_p3();

  /// Constructor with all parameters
  HepMcParticleLink_p3(
                       const unsigned short genEvtIndex,
                       const unsigned long id,
                       char evtColl );

  ///////////////////////////////////////////////////////////////////
  // Protected data:
  ///////////////////////////////////////////////////////////////////
protected:

  /// index of the @c HepMC::GenEvent holding the @c HepMC::GenParticle we
  /// are pointing to. This is the index in the @c McEventCollection.
  unsigned short m_mcEvtIndex{0};

  /// id of the @c HepMC::GenParticle we are pointing to.
  unsigned long m_id{0}; // TODO could this be an unsigned int instead?

  // identifier for the type of McEventCollection containing the particle (HS, PU,...)
  char m_evtColl{'a'};
};

///////////////////////////////////////////////////////////////////
/// Inline methods:
///////////////////////////////////////////////////////////////////

inline HepMcParticleLink_p3::HepMcParticleLink_p3() {}

inline
HepMcParticleLink_p3::HepMcParticleLink_p3(
                                           const unsigned short genEvtIndex,
                                           const unsigned long id,
                                           char evtColl) :
  m_mcEvtIndex( genEvtIndex ),
  m_id( id ),
  m_evtColl( evtColl )
{}

#endif //> GENERATOROBJECTSTPCNV_HEPMCPARTICLELINK_P3_H
