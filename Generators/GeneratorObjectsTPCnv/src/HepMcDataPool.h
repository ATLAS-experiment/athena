///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// HepMcDataPool.h
// Header file for a set of utilities for DataPool w/ HepMC classes
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef GENERATOROBJECTSATHENAPOOL_HEPMCDATAPOOL_H
#define GENERATOROBJECTSATHENAPOOL_HEPMCDATAPOOL_H

// HepMC / CLHEP includes
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#include "AthAllocators/DataPool.h"


namespace HepMC {

  struct DataPool {

    // Helpers for allocating HepMC objects from a DataPool.
    // But because HepMC3 keeps shared_ptr's to its objects, we need
    // to be careful here.
    //
    // First, the memory we get from the pool is actually owned by the pool,
    // so we don't want the shared_ptr's to actually delete anything.
    // We accomplish this by creating the shared_ptr's for particles and
    // vertices with null deleters.  (This isn't an issue for the GenEvent
    // objects, since we don't manage them what shared_ptr, but we do need
    // to be careful not to put them in an owning DataVector.)
    //
    // Second, before we create a shared_ptr with a pointer we've just
    // gotten from the DataPool, we need to be sure that there aren't any
    // other shared_ptr's to the same object --- otherwise, the behavior
    // is undefined.  (We hide the worst consequences of this by the fact
    // that we have no-op deleters, but it can still result in the weak
    // references in GenParticle mysteriously expiring.  See ATR-26790.)
    // So we need to clear the objects before that.  We could in principle
    // do that in the get* functions, but it's nicer to set up clear hooks
    // in the DataPool so that that happens when objects are returned
    // to the pool.  (And that way, we don't maintain allocated memory
    // from free objects in the pool.)

    struct ClearGenEvent
    {
      static void clear (HepMC::GenEvent* evt) { evt->clear(); }
    };
    ::DataPool<HepMC::GenEvent, ClearGenEvent> evt;
    HepMC::GenEvent* getGenEvent()
    {
      return evt.nextElementPtr();
    }

    struct ClearGenVertex
    {
      static void clear (HepMC::GenVertex* vtx) { *vtx = HepMC::GenVertex(); }
    };
    ::DataPool<HepMC::GenVertex, ClearGenVertex> vtx;
    HepMC::GenVertexPtr getGenVertex()
    {
      return HepMC::GenVertexPtr (vtx.nextElementPtr(), [](HepMC::GenVertex*){});
    }


    struct ClearGenParticle
    {
      static void clear (HepMC::GenParticle* part) { *part = HepMC::GenParticle(); }
    };
    ::DataPool<HepMC::GenParticle, ClearGenParticle> part;
    HepMC::GenParticlePtr getGenParticle()
    {
      return HepMC::GenParticlePtr (part.nextElementPtr(), [](HepMC::GenParticle*){});
    }

  };

} // end namespace HepMC

#endif // GENERATOROBJECTSATHENAPOOL_HEPMCDATAPOOL_H
