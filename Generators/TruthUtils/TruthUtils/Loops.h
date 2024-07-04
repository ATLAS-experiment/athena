/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS
#ifndef TRUTHUTILS_LOOPS_H
#define TRUTHUTILS_LOOPS_H
#include <algorithm>
#include <vector>
#include <memory>
namespace MC {
template <class Evt, class Prt, class Vtx>
class Loops  {
public:
    Loops() {}
    Loops(const Evt* evt) {
        findLoops(evt,true);
    }
    bool isLoop(const Prt& p) const {
        return std::find(m_loop_particles.begin(), m_loop_particles.end(), p) != m_loop_particles.end();
    }
    bool isLoop(const Vtx& v) const {
        return std::find(m_loop_vertices.begin(), m_loop_vertices.end(), v) != m_loop_vertices.end();
    }
    const std::vector<Prt>& loop_particles() const {
        return m_loop_particles;
    }
    const std::vector<Vtx>& loop_vertices() const {
        return m_loop_vertices;
    }
    int findLoops(const Evt* evt, bool force) {
        if (!evt) return -1;
        if (evt == m_evt &&  !force) return 0;
        m_evt = evt;
        m_loop_particles.clear();
        m_loop_vertices.clear();
        std::map<Prt, int> incycle;
        for (const auto & p: *m_evt) if (!p->end_vertex()||!p->production_vertex()) incycle[p] = -1;
            else incycle[p] = 0;
        size_t minincycle = m_evt->particles_size();
        for (;;) {
            size_t unknown = 0;
            for (const auto & p: *m_evt) {
                if (incycle[p] != 0) continue;
                unknown++;
                auto ev = p->end_vertex();
                if (ev) {
                    bool goodo = true;
                    for (auto& po: *ev) goodo = goodo && (incycle[po] == -1);
                    if (goodo) incycle[p] = -1;
                }
                auto pv = p->production_vertex();
                if (pv) {
                    bool goodi = true;
#ifdef HEPMC3
                    for (auto& pi: ev->particles_in()) goodi = goodi && (incycle[pi] == -1);
#else
                    for (auto ip = ev->particles_in_const_begin();
                         ip != ev->particles_in_const_end();
                         ++ip)
                    {
                      goodi = goodi && (incycle[*ip] == -1);
                    }
#endif
                    if (goodi) incycle[p] = -1;
                }
            }
            if (minincycle == unknown) break;
            minincycle = std::min(minincycle, unknown);
        }
        for (const auto & p: *m_evt) if (incycle[p] == 0) incycle[p] = 1;

        for (const auto & p: *m_evt) if (incycle[p] == 1) m_loop_particles.push_back(p);
#ifdef HEPMC3
        for (auto & v: m_evt->vertices()) {
#else
        for (auto iv = m_evt->vertices_begin(); iv != m_evt->vertices_end(); ++iv) {
            auto v = *iv;
#endif
            bool push = false;
#ifdef HEPMC3
            for ( auto& pin: v->particles_in()) if (incycle[pin] == 1) {
#else
            for ( auto ipin = v->particles_in_const_begin();
                  ipin != v->particles_in_const_end();
                  ++ipin)
              if (incycle[*ipin] == 1) {
#endif
                    push = true;
                    break;
                }
            if(!push) {
                for ( const auto& pou: *v) {
                    if (incycle[pou] == 1) {
                        push = true;
                        break;
                    }
                }
            }
            if (push)  m_loop_vertices.push_back(v);
        }
        return 0;
    }

private:
    const Evt* m_evt = nullptr;
    std::vector<Prt> m_loop_particles;
    std::vector<Vtx> m_loop_vertices;
};
}
#endif
#endif
