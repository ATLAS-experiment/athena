///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATOROBJECTSTPCNV_HEPMCPARTICLELINKCNV_P3_H
#define GENERATOROBJECTSTPCNV_HEPMCPARTICLELINKCNV_P3_H

// STL includes
#include <map>

// AthenaPoolCnvSvc includes
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

// GeneratorObjects includes
#include "GeneratorObjects/HepMcParticleLink.h"

// GeneratorObjectsAthenaPool includes
#include "GeneratorObjectsTPCnv/HepMcParticleLink_p3.h"

// Forward declaration
class MsgStream;

class HepMcParticleLinkCnv_p3 : public T_AthenaPoolTPCnvConstBase<
                                         HepMcParticleLink,
                                         HepMcParticleLink_p3
                                       >
{
public:
  using base_class::persToTrans;
  using base_class::transToPers;

  /** Default constructor:
   */
  HepMcParticleLinkCnv_p3() = default;


  /** Method creating the transient representation of @c HepMcParticleLink
   *  from its persistent representation @c HepMcParticleLink_p3
   */
  virtual void persToTrans( const HepMcParticleLink_p3* persObj,
                            HepMcParticleLink* transObj,
                            MsgStream &msg ) const override;

  /** Method creating the persistent representation @c HepMcParticleLink_p3
   *  from its transient representation @c HepMcParticleLink
   */
  virtual void transToPers( const HepMcParticleLink* transObj,
                            HepMcParticleLink_p3* persObj,
                            MsgStream &msg ) const override;
};


#endif //> GENERATOROBJECTSTPCNV_HEPMCPARTICLELINKCNV_P3_H
