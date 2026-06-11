/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/* Author: Andrii Verbytskyi andrii.verbytskyi@mpp.mpg.de */

#ifndef ATLASHEPMC_BARCODE_H
#define ATLASHEPMC_BARCODE_H
#include <type_traits>
#ifdef HEPMC3
#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"
#include "HepMC3/GenEvent.h"
#include "AtlasHepMC/AttributeNames.h"
#endif
namespace HepMC {
template <class T>
inline int barcode(const T* p){ return p->barcode(); }
inline int barcode(int p){ return p; }
#ifdef HEPMC3
template <class T, std::enable_if_t< !std::is_pointer<T>::value &&
                           !std::is_same<T, HepMC3::GenParticlePtr>::value &&
                           !std::is_same<T, HepMC3::ConstGenParticlePtr>::value &&
                           !std::is_same<T, HepMC3::GenVertexPtr>::value &&
                           !std::is_same<T, HepMC3::ConstGenVertexPtr>::value &&
                           !std::is_same<T, HepMC3::GenVertex>::value &&
                           !std::is_same<T, HepMC3::GenParticle>::value &&
                           !std::is_same<T, int>::value 
                        , bool > = true>
inline int barcode(const T& p){ return p.barcode();}

template <class T, std::enable_if_t< 
                           std::is_same<T, HepMC3::GenParticlePtr>::value ||
                           std::is_same<T, HepMC3::ConstGenParticlePtr>::value ||
                           std::is_same<T, HepMC3::GenVertexPtr>::value ||
                           std::is_same<T, HepMC3::ConstGenVertexPtr>::value
                        , bool > = true >
inline int barcode(const T& p) {
    if (!p) return 0;
    const HepMC3::GenEvent* e = p->parent_event();
    if (!e) return 0;
    std::shared_ptr<HepMC3::IntAttribute> barcode = e->attribute<HepMC3::IntAttribute>(HepMCStr::barcode, p->id());
    return barcode ? (barcode->value()) : p->id();
}

template <class T, std::enable_if_t< 
                           std::is_same<T, HepMC3::GenParticle>::value ||
                           std::is_same<T, HepMC3::GenVertex>::value
                        , bool > = true >
inline int barcode(const T& p) {
    const HepMC3::GenEvent* e = p.parent_event();
    if (!e) return 0;
    std::shared_ptr<HepMC3::IntAttribute> barcode = e->attribute<HepMC3::IntAttribute>(HepMCStr::barcode, p.id());
    return barcode ? (barcode->value()) : p.id();
}
#else
template <class T, std::enable_if_t<!std::is_pointer<T>::value &&
                                    !std::is_same<T,int>::value
                                    , bool> = true>  inline int barcode(const T& p){ return p.barcode();}
#endif

}
#endif
