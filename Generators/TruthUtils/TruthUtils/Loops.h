/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTHUTILS_LOOPS_H
#define TRUTHUTILS_LOOPS_H
#include "TruthUtils/MagicNumbers.h"

namespace MC {

/// Protection against loops in the truth record (MCTruthClassifier legacy).
/// Some generators (notably Sherpa) record a particle as a child of itself, or as its own ancestor, which would make
/// the ancestor searches in MCTruthClassifier run forever. The three methods below detect such loops; they are
/// used together. The functions work with any particle and vertex type that provides the xAOD::TruthParticle /
/// HepMC-like interface: `production_vertex()`, `end_vertex()`, `pdg_id()` on particles and `particles_out()` on
/// vertices. Pointers and smart pointers (e.g. HepMC3 `ConstGenParticlePtr`) are both accepted.

/// @brief Method 3: true if the parent and the child have the same production vertex (the Sherpa loop).
/// @param childOrigVtx production vertex of the child
/// @param parent       the candidate parent particle (must not be null)
template <class V, class T> bool TruthLoopDetectionMethod3(const V& childOrigVtx, const T& parent)
{
  const auto parentOrigVtx = parent->production_vertex();
  return parentOrigVtx && HepMC::is_same_vertex(parentOrigVtx, childOrigVtx);
}

/// @brief Method 1: true if the parent is among the children produced at the child's production vertex, i.e.
/// one of them is the same generator particle with the same PDG ID. Otherwise returns the result of Method 3.
/// Note: with several children the matching one is not necessarily the particle being classified.
/// @param childOrigVtx production vertex of the child
/// @param parent       the candidate parent particle (must not be null)
template <class V, class T> bool TruthLoopDetectionMethod1(const V& childOrigVtx, const T& parent)
{
  const int parentPDG = parent->pdg_id();
  for (const auto& aChild: childOrigVtx->particles_out()) {
    if (!aChild) continue;
    if (parentPDG == aChild->pdg_id() && HepMC::is_same_generator_particle(aChild, parent)) return true;
  }
  return TruthLoopDetectionMethod3(childOrigVtx, parent);
}

/// @brief Method 2: true if the parent's production vertex is the child's decay vertex and the child's production
/// vertex is the parent's decay vertex, i.e. V0 -> parent -> V1 -> ... -> V2 -> child -> V3 with V3 == V0 and V1 == V2.
/// Null particles are allowed and give false unless both vertices are null.
/// @param child  the particle being classified
/// @param parent the candidate parent particle
template <class T1, class T2> bool TruthLoopDetectionMethod2(const T1& child, const T2& parent)
{
  decltype(child->production_vertex()) child_prdVtx{};
  decltype(child->end_vertex()) child_endVtx{};
  if (child) {
    child_prdVtx = child->production_vertex();
    child_endVtx = child->end_vertex();
  }
  decltype(parent->production_vertex()) parent_prdVtx{};
  decltype(parent->end_vertex()) parent_endVtx{};
  if (parent) {
    parent_prdVtx = parent->production_vertex();
    parent_endVtx = parent->end_vertex();
  }
  return (child_endVtx == parent_prdVtx && child_prdVtx == parent_endVtx);
}
}

#ifndef XAOD_ANALYSIS
#include <algorithm>
#include <map>
#include <vector>
#include <memory>

namespace MC {
template <class Evt, class Prt, class Vtx>
class Loops  {
public:

    /// Default constructor
    Loops() {}

    /// Constructor passing the event; immediately index the event that is passed.
    Loops(const Evt* evt) {
        findLoops(evt,true);
    }

    /// Is this particle in a loop?
    bool isLoop(const Prt& p) const {
        return std::find(m_loop_particles.begin(), m_loop_particles.end(), p) != m_loop_particles.end();
    }

    /// Is this vertex in a loop?
    bool isLoop(const Vtx& v) const {
        return std::find(m_loop_vertices.begin(), m_loop_vertices.end(), v) != m_loop_vertices.end();
    }

    /// Accessor: return the full list of particles in loops
    const std::vector<Prt>& loop_particles() const {
        return m_loop_particles;
    }

    /// Accessor: return the full list of vertices in loops
    const std::vector<Vtx>& loop_vertices() const {
        return m_loop_vertices;
    }

    /// Function that does the work to identify loops
    /// Pass the event of interest. 'force' allows the event to be re-searched even if it was previously searched.
    int findLoops(const Evt* evt, bool force) {
        // If no event is passed, return -1 (an error code, basically)
        if (!evt) return -1;
        // If we have already indexed the event and are not forced to do it again, return
        if (evt == m_evt &&  !force) return 0;

        // Update the local event pointer. This is now the most recent event that has been indexed.
        m_evt = evt;
        // Clear the lists of particles and vertices. These will now refer to this event.
        m_loop_particles.clear();
        m_loop_vertices.clear();

        // Create a map, keeping track of whether particles are in loops.
        // 0 for "unknown", 1 for "in a loop", -1 for "not in a loop"
        std::map<Prt, int> incycle;

        // Easiest case to deal with: if a particle has no production or end vertex, it is not in a loop
        for (const auto & p: *m_evt){
            if (!p->end_vertex()||!p->production_vertex()){
              incycle[p] = -1;
            } else {
              incycle[p] = 0;
            }
        }

        // Now begin iterating through particles to try to find loops
        size_t minincycle = m_evt->particles_size();
        for (;;) {
            // Number of particles still unknown in the current iteration
            size_t unknown = 0;
            for (const auto & p: *m_evt) {

                // If the particle was already identified as in/not in a loop, skip it
                if (incycle[p] != 0) continue;
                // Otherwise it is unknown on this iteration
                unknown++;

                // Start from the end vertex of the particle
                auto ev = p->end_vertex();
                if (ev) {
                    // If the end vertex of a particle exclusively has particles not in loops
                    // then this particle cannot be in a loop either
                    bool goodo = true;
                    for (auto& po: *ev) goodo = goodo && (incycle[po] == -1);
                    if (goodo) incycle[p] = -1;
                }

                // Now start from the production vertex of the particle
                auto pv = p->production_vertex();
                if (pv) {
                    // If the production vertex of a particle exclusively has particles not
                    // in loops, then this particle cannot be in a loop either
                    bool goodi = true;
                    for (auto& pi: pv->particles_in()) goodi = goodi && (incycle[pi] == -1);
                    if (goodi) incycle[p] = -1;
                }
            }
            // If the number of unknown particles has not changed, we are done iterating
            if (minincycle == unknown) break;
            // If the number of unknown particles has changed, update and iterate again
            minincycle = std::min(minincycle, unknown);
        } // Outer loop - iterate until all particles have been dealt with

        // Any remaining particles that have not been identified are a part of a loop
        // Add them to our list of looping particles
        for (const auto & p: *m_evt){
          if (incycle[p] == 0) incycle[p] = 1;
          if (incycle[p] == 1) m_loop_particles.push_back(p);
        }

        // Now loop over all vertices.
        for (auto & v: m_evt->vertices()) {
            bool push = false;
            // First check incoming particles.
            // If any incoming are in loops, consider this vertex to be in a loop.
            for ( auto& pin: v->particles_in()){
                if (incycle[pin] == 1) {
                      push = true;
                      break;
                }
            } // End of loop over incoming particles
            // In case we have not yet identified the vertex as being in a loop
            // then check the outgoing particles from the vertex.
            // If any outgoing are in loops, consider this vertex to be in a loop.
            if(!push) {
                for ( const auto& pou: *v) {
                    if (incycle[pou] == 1) {
                        push = true;
                        break;
                    }
                }
            }
            // Update the records if this vertex is in a loop.
            if (push) m_loop_vertices.push_back(v);
        } // End of loop over vertices

        // All finished. Return success.
        return 0;
    } // End of the findLoops function

private:

    /// Local pointer to the event passed in the constructor or most recent call to findLoops
    const Evt* m_evt = nullptr;

    /// List of all particles in m_evt that are in loops
    std::vector<Prt> m_loop_particles;

    /// List of all vertices in m_evt that are in loops
    std::vector<Vtx> m_loop_vertices;
};
}
#endif
#endif
