/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRUTHUTILS_DECAYPRODUCTS_H
#define TRUTHUTILS_DECAYPRODUCTS_H
#include <vector>
#include <type_traits>
#include <algorithm>
#include <map>
#include <vector>
template <class T> class DecayBase {
public:
    void count(const int c) { m_pids[c]++; m_apids[std::abs(c)]++; m_size++;}
    int pd(int i) const { return m_pids.count(i)?m_pids.at(i):0; }
    int pd(int i1, int i2) const { int ret = 0; for ( auto & a: m_pids) if (i1 <= a.first && a.first <= i2) ret += a.second; return ret; }
    int apd(int i1, int i2) const { int ret = 0; for ( auto & a: m_apids) if (i1 <= a.first && a.first <= i2) ret += a.second; return ret; }
    int apd(int i) const { return m_apids.count(i)?m_apids.at(i):0; }
    int pd(const std::vector<int>& i) const { int ret = 0; for ( auto ii: i) ret += pd(ii); return ret;}
    int apd(const std::vector<int>& i) const { int ret = 0; for ( auto ii: i) ret += apd(ii); return ret; }
    size_t size() const { return m_size; }
    int apply(bool (*func)(const int&)) const { int ret = 0; for ( auto & a: m_pids) if (func(a.first)) ret+=a.second; return ret;  }
    std::map<int,int>  m_pids;
    std::map<int,int> m_apids;
    size_t m_size;
};
#ifdef ATLASHEPMC_GENEVENT_H
#ifdef ATLASHEPMC_GENPARTICLE_H
    template <class Y,std::enable_if_t<std::is_same<Y, HepMC::GenParticlePtr>::value ||std::is_same<Y, HepMC::ConstGenParticlePtr>::value, bool > = true >
    DecayBase<Y> DecayProducts(const Y& p) {
        DecayBase<Y> t;
        auto v = p->end_vertex();
        if (v) for (auto prod: *v) if (prod) t.count(prod->pdg_id());
        return t;
    }
#endif
#ifdef ATLASHEPMC_GENVERTEX_H
    template <class Y,std::enable_if_t<std::is_same<Y, HepMC::GenVertexPtr>::value ||std::is_same<Y, HepMC::ConstGenVertexPtr>::value, bool > = true>
    DecayBase<Y> DecayProducts(const Y& p) {
        DecayBase<Y> t;
        if (p) for (auto prod: *p) if (prod) t.count(prod->pdg_id());
        return t;        
    }
#endif
#endif
#ifdef XAODTRUTH_TRUTHPARTICLE_H
    template <class Y,std::enable_if_t<std::is_same<Y, xAOD::TruthParticle>::value, bool > = true>
    DecayBase<Y> DecayProducts(const Y& p) {
        DecayBase<Y> t;
        auto v = p.end_vertex();
        if (v) for (auto prod: v->particles_out()) if (prod) t.count(prod->pdg_id());
        return t;        
    }
    template <class Y,std::enable_if_t<std::is_same<Y, xAOD::TruthParticle*>::value || std::is_same<Y, xAOD::TruthParticle const*>::value , bool > = true>
    DecayBase<Y> DecayProducts(const Y& p) {
        DecayBase<Y> t;
        auto v = p->end_vertex();
        if (v) for (auto prod: v->particles_out()) if (prod) t.count(prod->pdg_id());
        return t;        
    }
    template <class Y,std::enable_if_t<std::is_same<Y, std::vector<const xAOD::TruthParticle*> >::value, bool > = true>
    DecayBase<Y> DecayProducts(const Y& p) {
        DecayBase<Y> t;
        for (auto prod: p) if (prod) t.count(prod->pdg_id());
        return t;        
    }
#endif
#ifdef XAODTRUTH_TRUTHVERTEX_H
    template <class Y,std::enable_if_t<std::is_same<Y, xAOD::TruthVertex*>::value || std::is_same<Y, xAOD::TruthVertex const*>::value , bool > = true>
    DecayBase<Y> DecayProducts(const Y& p) {
        DecayBase<Y> t;
        if (p) for (auto prod: p->particles_out()) if (prod) t.count(prod->pdg_id());
        return t;        
    }
#endif
#endif
