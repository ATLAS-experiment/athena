/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarExampleTools/VariantExampleTool.h>

//
// method implementations
//

namespace columnar
{
  VariantExampleTool ::
  VariantExampleTool (const std::string& name)
    : AsgTool (name)
  {}



  StatusCode VariantExampleTool ::
  initialize ()
  {
    // give the base class a chance to initialize the column accessor
    // backends
    ANA_CHECK (initializeColumns());
    return StatusCode::SUCCESS;
  }



  void VariantExampleTool ::
  callSingleEvent (ElectronRange electrons, MuonRange muons) const
  {
    // First copy all the wanted particles into a vector of variant
    // objects. These then no longer care whether they are electrons or
    // muons, they can be either. Effectively these acts like a "view"
    // container, i.e. each entry still refers to the original container
    // and there isn't a new aux/column store for the data being
    // accessed.
    //
    // If you use variant object ids, there is a fair chance you are
    // using this kind of pattern, as there aren't a lot of ways to get
    // "variant" objects.
    //
    // Please note that in columnar mode it would be more efficient to
    // embed this whole function inside `callEvents` and simply reset
    // the vector at the start of each event. For a simple tool like
    // this that saves about 33% of total execution time, but for
    // purposes of demonstration and testing the current setup is
    // simpler.
    std::vector<ObjectId<MyVariantDef>> variantParticles;
    variantParticles.reserve (electrons.size() + muons.size());
    for (auto electron : electrons)
      variantParticles.push_back(electron);
    for (auto muon : muons)
      variantParticles.push_back(muon);

    for (auto& variantParticle : variantParticles)
    {
      // It is possible to check whether a given column is available,
      // just as for regular column accessors. For variant objects that
      // happens on a per-object basis, as some columns may not be
      // defined for all objects. This is more to illustrate that it can
      // be done, but will never trigger. Also, we didn't mark the
      // column as optional, which means that at least in columnar mode the
      // column is guaranteed to be there.
      if (!ptAcc.isAvailable(variantParticle))
      {
        ANA_MSG_WARNING ("pt is not available for " << variantParticle);
        throw std::runtime_error("pt is not available");
      }
    }

    // sort the combined electron-muon vector by PT.
    std::sort (variantParticles.begin(), variantParticles.end(),
      [&](const auto& a, const auto& b) {
        return ptAcc(a) > ptAcc(b);
      });

    // attach the PT rank decoration to each variant particle
    for (std::size_t rank = 0; rank < variantParticles.size(); ++rank)
    {
      ptRankDec(variantParticles[rank]) = rank;
    }

    // sort the combined electron-muon vector by |eta|.
    std::sort (variantParticles.begin(), variantParticles.end(),
      [&](const auto& a, const auto& b) {
        return std::abs (etaAcc(a)) < std::abs (etaAcc(b));
      });

    // attach the eta rank decoration to each variant particle
    for (std::size_t rank = 0; rank < variantParticles.size(); ++rank)
    {
      // an example of how to convert to a specific container and do
      // something just for that container. in this case we are just
      // applying the same decoration under a different name, but it is
      // hopefully clear how that could be utilized otherwise.
      if (auto castObject = variantParticles[rank].tryGetVariant<ContainerId::electron>())
        etaRankSpecialDec(*castObject) = rank;
    }
  }



  void VariantExampleTool ::
  callEvents (EventContextRange events) const
  {
    std::vector<ObjectId<MyVariantDef>> variantParticles;

    // loop over all events and particles.  note that this is
    // deliberately looping by value, as the ID classes are very small
    // and can be copied cheaply.  this could have also been written as
    // a single loop over all particles in the event range, but I chose
    // to split it up into two loops as most tools will need to do some
    // per-event things, e.g. retrieve `EventInfo`.
    for (columnar::EventContextId event : events)
    {
      // variantParticles.clear();
      // auto electrons = electronsHandle(event);
      // auto muons = muonsHandle(event);
      // for (auto electron : electrons)
      //   variantParticles.push_back(electron);
      // for (auto muon : muons)
      //   variantParticles.push_back(muon);
      // std::sort (variantParticles.begin(), variantParticles.end(),
      //   [&](const auto& a, const auto& b) {
      //     return ptAcc(a) > ptAcc(b);
      //   });
      // for (std::size_t rank = 0; rank < variantParticles.size(); ++rank)
      // {
      //   ptRankDec(variantParticles[rank]) = rank;

      //   // an example of how to convert to a specific 
      //   if (auto castObject = variantParticles[rank].tryGetVariant<ContainerId::electron>())
      //     ptRankSpecialDec(*castObject) = rank;
      // }
      callSingleEvent (electronsHandle(event), muonsHandle (event));
    }
  }
}
