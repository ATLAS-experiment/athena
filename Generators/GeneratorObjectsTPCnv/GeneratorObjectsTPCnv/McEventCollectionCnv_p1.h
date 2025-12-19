///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// McEventCollectionCnv_p1.h
// Header file for class McEventCollectionCnv_p1
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef GENERATOROBJECTSTPCNV_MCEVENTCOLLECTIONCNV_P1_H
#define GENERATOROBJECTSTPCNV_MCEVENTCOLLECTIONCNV_P1_H

#include <sstream>

// STL includes
#ifndef HEPMC3
# ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wkeyword-macro"
# endif
# if __GNUC__ >= 16
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wkeyword-macro"
# endif
# define private public
# define protected public
#endif
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#ifndef HEPMC3
# undef private
# undef protected
# ifdef __clang__
#  pragma clang diagnostic pop
# endif
# if __GNUC__ >= 16
#  pragma GCC diagnostic pop
# endif
#endif
#include "GeneratorObjects/McEventCollection.h"

// AthenaPoolCnvSvc includes
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

// GeneratorObjectsTPCnv includes
#include "GeneratorObjectsTPCnv/McEventCollection_p1.h"

// Forward declaration
class MsgStream;
class McEventCollectionCnv_p1 : public T_AthenaPoolTPCnvBase<
                                          McEventCollection,
                                          McEventCollection_p1
                                       >
{
  typedef T_AthenaPoolTPCnvBase<McEventCollection,
                                McEventCollection_p1> Base_t;

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
 public:

  /** Default constructor:
   */
  McEventCollectionCnv_p1();

  /** Copy constructor
   */
  McEventCollectionCnv_p1( const McEventCollectionCnv_p1& rhs );

  /** Assignement operator
   */
  McEventCollectionCnv_p1& operator=( const McEventCollectionCnv_p1& rhs );

  /** Destructor
   */
  virtual ~McEventCollectionCnv_p1();

  /** Method creating the transient representation of @c McEventCollection
   *  from its persistent representation @c McEventCollection_p1
   */
  virtual void persToTrans( const McEventCollection_p1* persObj,
                            McEventCollection* transObj,
                            MsgStream& msg ) ;

  /** Method creating the persistent representation @c McEventCollection_p1
   *  from its transient representation @c McEventCollection
   */
  virtual void transToPers( const McEventCollection* transObj,
                            McEventCollection_p1* persObj,
                            MsgStream& msg ) ;
};
#endif //> GENERATOROBJECTSTPCNV_MCEVENTCOLLECTIONCNV_P1_H
