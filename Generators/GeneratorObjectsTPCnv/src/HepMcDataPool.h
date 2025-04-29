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
#ifndef HEPMC3
# ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wkeyword-macro"
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
# pragma clang diagnostic pop
# endif
#endif

#include "AthAllocators/DataPool.h"

#ifndef HEPMC3

// specialization of the destruction functions for our various DataPools
// these specializations are needed because we have to work-around the
// 'shared' ownership of particles and vertices by both GenEvent and the
// various DataPool<Xyz>.
namespace SG {

  template<>
  inline void
  ArenaAllocatorBase::destroy_fcn<HepMC::GenParticle>(ArenaAllocatorBase::pointer p)
  {
    HepMC::GenParticle* part = reinterpret_cast<HepMC::GenParticle*>(p);
    part->m_production_vertex = 0;
    part->m_end_vertex = 0;
    part->~GenParticle();
  }

  template<>
  inline void
  ArenaAllocatorBase::destroy_fcn<HepMC::GenVertex>(ArenaAllocatorBase::pointer p)
  {
    HepMC::GenVertex* vtx = reinterpret_cast<HepMC::GenVertex*>(p);
    vtx->m_event = 0;
    vtx->m_particles_in.clear();
    vtx->m_particles_out.clear();
    vtx->~GenVertex();
  }

  template<>
  inline void
  ArenaAllocatorBase::destroy_fcn<HepMC::GenEvent>(ArenaAllocatorBase::pointer p)
  {
    HepMC::GenEvent* evt = reinterpret_cast<HepMC::GenEvent*>(p);
    evt->m_particle_barcodes.clear();
    evt->m_vertex_barcodes.clear();
    delete evt->m_pdf_info; evt->m_pdf_info = 0;
    evt->~GenEvent();
  }
} // end namespace SG

#endif  

namespace HepMC {

  struct DataPool {

#ifdef HEPMC3
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
#else
    typedef ::DataPool<HepMC::GenEvent> GenEvtPool_t;
    /// an arena of @c HepMC::GenEvent for efficient object instantiation
    GenEvtPool_t evt;

    typedef ::DataPool<HepMC::GenVertex> GenVtxPool_t;
    /// an arena of @c HepMC::GenVertex for efficient object instantiation
    GenVtxPool_t vtx;

    typedef ::DataPool<HepMC::GenParticle> GenPartPool_t;
    /// an arena of @c HepMC::GenParticle for efficient object instantiation
    GenPartPool_t part;

    HepMC::GenEvent* getGenEvent()
    {
      return evt.nextElementPtr();
    }

    HepMC::GenVertexPtr getGenVertex()
    {
      return vtx.nextElementPtr();
    }

    HepMC::GenParticlePtr getGenParticle()
    {
      return part.nextElementPtr();
    }
#endif

  };

} // end namespace HepMC

#endif // GENERATOROBJECTSATHENAPOOL_HEPMCDATAPOOL_H
