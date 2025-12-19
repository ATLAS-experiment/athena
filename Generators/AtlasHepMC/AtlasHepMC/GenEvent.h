/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/* Author: Andrii Verbytskyi andrii.verbytskyi@mpp.mpg.de */

#ifndef ATLASHEPMC_GENEVENT_H
#define ATLASHEPMC_GENEVENT_H
#ifdef HEPMC3
#include "HepMC3/GenEvent.h"
#include "HepMC3/GenHeavyIon.h"
#include "HepMC3/GenPdfInfo.h"
#include "HepMC3/PrintStreams.h"
#include "AtlasHepMC/Barcode.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/SimpleVector.h"

#include <unordered_map>

namespace HepMC3 {
inline std::vector<HepMC3::GenParticlePtr>::const_iterator  begin(HepMC3::GenEvent& e) { return e.particles().begin(); }
inline std::vector<HepMC3::GenParticlePtr>::const_iterator  end(HepMC3::GenEvent& e) { return e.particles().end(); }
inline std::vector<HepMC3::ConstGenParticlePtr>::const_iterator  begin(const HepMC3::GenEvent& e) { return e.particles().begin(); }
inline std::vector<HepMC3::ConstGenParticlePtr>::const_iterator  end(const HepMC3::GenEvent& e) { return e.particles().end(); }
}

namespace HepMC {
using Print=HepMC3::Print;
using GenHeavyIon=HepMC3::GenHeavyIon;
using GenEvent=HepMC3::GenEvent;

class ShortEventAttribute : public HepMC3::Attribute {
public:
    ShortEventAttribute():HepMC3::Attribute(){}
    ShortEventAttribute(const HepMC3::GenEvent* e):HepMC3::Attribute(){ from_event(e); }

    bool from_event(const HepMC3::GenEvent* e){
      NUP=e->particles().size();
      resize();
      XWGTUP = e->weights().size() ? e->weights()[0] : 1.0;
      auto A_signal_process_id=e->attribute<HepMC3::IntAttribute>("signal_process_id");
      IDPRUP = A_signal_process_id?A_signal_process_id->value() : 0;
      auto A_event_scale=e->attribute<HepMC3::DoubleAttribute>("event_scale");
      SCALUP = A_event_scale? A_event_scale->value():0;
      auto A_alphaQCD=e->attribute<HepMC3::DoubleAttribute>("alphaQCD");
      AQCDUP = A_alphaQCD? A_alphaQCD->value():0;
      auto A_alphaQED=e->attribute<HepMC3::DoubleAttribute>("alphaQED");
      AQEDUP = A_alphaQED? A_alphaQED->value():0;

      for ( int i = 0; i < NUP; ++i ){
        PUP[i][0] = e->particles().at(i)->momentum().px();
        PUP[i][1] = e->particles().at(i)->momentum().py();
        PUP[i][2] = e->particles().at(i)->momentum().pz();
        PUP[i][3] = e->particles().at(i)->momentum().e();
        PUP[i][4] = e->particles().at(i)->momentum().m();
        IDUP[i] = e->particles().at(i)->pdg_id();
        auto pv = e->particles().at(i)->production_vertex();
        auto ev = e->particles().at(i)->end_vertex();
        if (pv && ev ) ISTUP[i] = 2;
        if (pv && !ev ) ISTUP[i] = 1;
        if (e->particles().at(i)->status() == 4 || !pv ) ISTUP[i] = -1;
        auto flow1 = e->particles().at(i)->attribute<HepMC3::IntAttribute>("flow1");
        auto flow2 = e->particles().at(i)->attribute<HepMC3::IntAttribute>("flow2");
        ICOLUP[i].first = flow1 ? flow1->value() : 0;
        ICOLUP[i].second = flow2 ? flow2->value() : 0;
        int l = 0,h = 0;
        if (pv) for (auto p: pv->particles_in()) { l= std::min(l,p->id()); h=std::max(h,p->id());}
        MOTHUP[i].first = h;
        MOTHUP[i].second = l;
      }
      return true;
   }

    bool from_string(const std::string &att) override {
      std::istringstream iss(att);
      iss >> NUP;
      iss >> IDPRUP; 
      iss >> XWGTUP;
      iss >> SCALUP;
      iss >> AQEDUP;
      iss >> AQCDUP;
      //assume input is a trusted source
      //coverity[tainted_data] 
      resize();
      for ( int i = 0; i < NUP; ++i ){
        iss >>  IDUP[i];
        iss >>  ISTUP[i];
        iss >>  MOTHUP[i].first;
        iss >>  MOTHUP[i].second;
        iss >>  ICOLUP[i].first;
        iss >>  ICOLUP[i].second;
        iss >>  PUP[i][0];
        iss >>  PUP[i][1];
        iss >>  PUP[i][2];
        iss >>  PUP[i][3];
        iss >> PUP[i][4];
        iss >>  VTIMUP[i];
        iss >>  SPINUP[i];
      }
        set_is_parsed(true);
        return true;
    }

    bool to_string(std::string &fl) const  override {
      std::ostringstream file;
      file << " " << std::setw(4) << NUP
           << " " << std::setw(6) << IDPRUP
           << " " << std::setw(14) << XWGTUP
           << " " << std::setw(14) << SCALUP
           << " " << std::setw(14) << AQEDUP
           << " " << std::setw(14) << AQCDUP << "\n";
      for ( int i = 0; i < NUP; ++i )
        file << " " << std::setw(8) << IDUP[i]
             << " " << std::setw(2) << ISTUP[i]
             << " " << std::setw(4) << MOTHUP[i].first
             << " " << std::setw(4) << MOTHUP[i].second
             << " " << std::setw(4) << ICOLUP[i].first
             << " " << std::setw(4) << ICOLUP[i].second
             << " " << std::setw(14) << PUP[i][0]
             << " " << std::setw(14) << PUP[i][1]
             << " " << std::setw(14) << PUP[i][2]
             << " " << std::setw(14) << PUP[i][3]
             << " " << std::setw(14) << PUP[i][4]
             << " " << std::setw(1) << VTIMUP[i]
             << " " << std::setw(1) << SPINUP[i] << std::endl;
       fl+=file.str();
       return true;
    }

  void resize() {
    IDUP.resize(NUP);
    ISTUP.resize(NUP);
    MOTHUP.resize(NUP);
    ICOLUP.resize(NUP);
    PUP.resize(NUP, std::vector<double>(5));
    VTIMUP.resize(NUP);
    SPINUP.resize(NUP);
  }

  int NUP=0;
  int IDPRUP=0;
  double XWGTUP=0;
  double SCALUP=0;
  double AQEDUP=0;
  double AQCDUP=0;
  std::vector<long> IDUP{};
  std::vector<int> ISTUP{};
  std::vector< std::pair<int,int> > MOTHUP{};
  std::vector< std::pair<int,int> > ICOLUP{};
  std::vector< std::vector<double> > PUP{};
  std::vector<double> VTIMUP{};
  std::vector<double> SPINUP{};
};


class GenEventBarcodes : public HepMC3::Attribute
{
public:
  virtual bool from_string(const std::string & /*att*/) override { return false; }
  virtual bool to_string(std::string &att) const override
  {
    att =  "GenEventBarcodes";
    return true;
  }
  ConstGenVertexPtr barcode_to_vertex (int id) const {
    auto it = m_vertexBC.find (id);
    if (it != m_vertexBC.end()) return it->second;
    return nullptr;
  }
  GenVertexPtr barcode_to_vertex (int id) {
    auto it = m_vertexBC.find (id);
    if (it != m_vertexBC.end()) return it->second;
    return nullptr;
  }
  ConstGenParticlePtr barcode_to_particle (int id) const {
    auto it = m_particleBC.find (id);
    if (it != m_particleBC.end()) return it->second;
    return nullptr;
  }
  GenParticlePtr barcode_to_particle (int id) {
    auto it = m_particleBC.find (id);
    if (it != m_particleBC.end()) return it->second;
    return nullptr;
  }

  void add (GenVertexPtr p) {
    if (!p) return;
    auto barcode = p->attribute<HepMC3::IntAttribute>("barcode");
    if (barcode) {
      m_vertexBC[barcode->value()] = std::move(p);
    }
  }

  void remove (GenVertexPtr p) {
    if (!p) return;
    auto barcode = p->attribute<HepMC3::IntAttribute>("barcode");
    if (barcode) {
      auto it = m_vertexBC.find (barcode->value());
      if (it != m_vertexBC.end()) {
        m_vertexBC.erase (it);
      }
    }
  }

  void add (GenParticlePtr p) {
    if (!p) return;
    auto barcode = p->attribute<HepMC3::IntAttribute>("barcode");
    if (barcode) {
      m_particleBC[barcode->value()] = std::move(p);
    }
  }

  void remove (GenParticlePtr p) {
    if (!p) return;
    auto barcode = p->attribute<HepMC3::IntAttribute>("barcode");
    if (barcode) {
      auto it = m_particleBC.find (barcode->value());
      if (it != m_particleBC.end()) {
        m_particleBC.erase (it);
      }
    }
  }
  
  std::map<int, ConstGenVertexPtr> barcode_to_vertex_map() const {
    std::map<int, ConstGenVertexPtr> ret;
    for (const auto &bcvertpair: m_vertexBC)
      ret.insert({bcvertpair.first,std::const_pointer_cast<const HepMC3::GenVertex>(bcvertpair.second)});
    return ret;
   }
  std::map<int, ConstGenParticlePtr> barcode_to_particle_map() const {
    std::map<int, ConstGenParticlePtr> ret;
    for (const auto &bcpartpair: m_particleBC)
      ret.insert({bcpartpair.first,std::const_pointer_cast<const HepMC3::GenParticle>(bcpartpair.second)});
    return ret;
   }
  std::map<int,int> id_to_barcode_map() const {
    std::map<int, int> ret;
    for (const auto &bcvertpair: m_vertexBC) ret.insert({bcvertpair.second->id(),bcvertpair.first});
    for (const auto &bcpartpair: m_particleBC) ret.insert({bcpartpair.second->id(),bcpartpair.first});
    return ret;
   }


  void fillAttribute(GenEvent* e) {
    const auto eventAttributes = e->attributes(); // this makes a copy
    const auto barcodeAttributeIt = eventAttributes.find("barcode");
    const bool hasBarcodeAttribute = barcodeAttributeIt != eventAttributes.end();

    const auto &particles = e->particles();
    for (size_t i = 1; i <= particles.size(); i++) {
      if (hasBarcodeAttribute && barcodeAttributeIt->second.count(i) > 0) {
        const auto &ptr = barcodeAttributeIt->second.at(i);
        if (ptr->is_parsed()) {
          m_particleBC[static_cast<HepMC3::IntAttribute*>(ptr.get())->value()] = ptr->particle();
        }
        else {
          m_particleBC[std::atoi(ptr->unparsed_string().c_str())] = ptr->particle();
        }
      } else {
        m_particleBC[i] = particles[i-1];
      }
    }
    const auto &vertices = e->vertices();
    for (size_t i = 1; i <= vertices.size(); i++) {
      if (hasBarcodeAttribute && barcodeAttributeIt->second.count(-i) > 0) {
        const auto &ptr = barcodeAttributeIt->second.at(-i);
        if (ptr->is_parsed()) {
          m_vertexBC[static_cast<HepMC3::IntAttribute*>(ptr.get())->value()] = ptr->vertex();
        }
        else {
          m_vertexBC[std::atoi(ptr->unparsed_string().c_str())] = ptr->vertex();
        }
      } else {
        m_vertexBC[-i] = vertices[i-1];
      }
    }
    set_is_parsed(true);
  }

private:
  std::unordered_map<int, GenVertexPtr> m_vertexBC;
  std::unordered_map<int, GenParticlePtr> m_particleBC;
};

/// @brief Attribute for linking GenParticles between GenEvents
///
/// Variation of the AssociatedParticle Attribute, which allows
/// GenParticles to be linked between GenEvents. For transient use
/// only as the from_string(...) method is not defined
class ShadowParticle : public HepMC3::Attribute {
public:

  /** @brief Default constructor */
  ShadowParticle() {}

  /** @brief Constructor initializing attribute value */
  ShadowParticle(ConstGenParticlePtr p)
    : Attribute(), m_shadow(p) {}

  /** @brief Implementation of Attribute::from_string */
  virtual bool from_string(const std::string &) override { return false; }

  /** @brief Implementation of Attribute::to_string */
  virtual bool to_string(std::string &att) const override
  {
    att =  "ShadowParticle";
    return true;
  }
  /** @brief get a pointer to the shadow particle. */
  ConstGenParticlePtr value() const {
    return m_shadow;
  }

private:

  ConstGenParticlePtr m_shadow; ///< The shadow particle.
};

inline bool set_ll_event_number(HepMC3::GenEvent* e, long long int num){
  e->add_attribute("long_long_event_number", std::make_shared<HepMC3::LongLongAttribute>(num));
  return true;
}
inline long long int get_ll_event_number(const HepMC3::GenEvent* e){
  auto at = e->attribute<HepMC3::LongLongAttribute>("long_long_event_number");
  return at?at->value():e->event_number();
}

inline std::map<std::string, std::size_t> weights_map(const HepMC3::GenEvent* e) {
  std::map<std::string, std::size_t>  ret;
  auto run = e->run_info();
  if (!run) return ret;
  std::vector<std::string> names = run->weight_names();
  for (const auto& name: names) ret[name] = run->weight_index(name);
  return ret;
}

inline std::vector<HepMC3::GenParticlePtr>::const_iterator  begin(HepMC3::GenEvent& e) { return e.particles().begin(); }
inline std::vector<HepMC3::GenParticlePtr>::const_iterator  end(HepMC3::GenEvent& e) { return e.particles().end(); }
inline std::vector<HepMC3::ConstGenParticlePtr>::const_iterator  begin(const HepMC3::GenEvent& e) { return e.particles().begin(); }
inline std::vector<HepMC3::ConstGenParticlePtr>::const_iterator  end(const HepMC3::GenEvent& e) { return e.particles().end(); }

inline GenEvent* newGenEvent(const int signal_process_id, const int event_number ) { // TODO Update event_number to long long int?
    GenEvent* e= new GenEvent();
    std::shared_ptr<HepMC3::IntAttribute> signal_process_id_A = std::make_shared<HepMC3::IntAttribute>(signal_process_id);
    e->add_attribute("signal_process_id",signal_process_id_A);
    e->add_attribute("barcodes", std::make_shared<GenEventBarcodes>());
    e->set_event_number(event_number);
    return e;
}

inline GenEvent* copyemptyGenEvent(const GenEvent* inEvt) {
  GenEvent* e= new GenEvent();
  e->set_event_number(inEvt->event_number());
  e->weights()=inEvt->weights();
  auto a_mpi = inEvt->attribute<HepMC3::IntAttribute>("mpi"); 
  if (a_mpi) e->add_attribute("mpi",std::make_shared<HepMC3::IntAttribute>(*a_mpi));
  auto a_signal_process_id = inEvt->attribute<HepMC3::IntAttribute>("signal_process_id");
  if (a_signal_process_id) e->add_attribute("signal_process_id",std::make_shared<HepMC3::IntAttribute>(*a_signal_process_id));
  auto a_event_scale = inEvt->attribute<HepMC3::DoubleAttribute>("event_scale");
  if (a_event_scale) e->add_attribute("event_scale",std::make_shared<HepMC3::DoubleAttribute>(*a_event_scale));
  auto a_alphaQCD = inEvt->attribute<HepMC3::DoubleAttribute>("alphaQCD");
  if (a_alphaQCD) e->add_attribute("alphaQCD",std::make_shared<HepMC3::DoubleAttribute>(*a_alphaQCD));
  auto a_alphaQED = inEvt->attribute<HepMC3::DoubleAttribute>("alphaQED");
  if (a_alphaQED) e->add_attribute("alphaQED",std::make_shared<HepMC3::DoubleAttribute>(*a_alphaQED));
  auto a_pi = inEvt->pdf_info(); 
  if (a_pi) e->set_pdf_info(std::make_shared<HepMC3::GenPdfInfo>(*a_pi));
  auto a_hi = inEvt->heavy_ion(); 
  if (a_hi) e->set_heavy_ion(std::make_shared<HepMC3::GenHeavyIon>(*a_hi));
  auto a_random_states = inEvt->attribute<HepMC3::VectorLongIntAttribute>("random_states");
  if (a_random_states) e->add_attribute("random_states",std::make_shared<HepMC3::VectorLongIntAttribute>(*a_random_states));
  e->add_attribute("barcodes", std::make_shared<GenEventBarcodes>());
  return e;
}

inline void fillBarcodesAttribute(GenEvent* e) {
  auto barcodes = e->attribute<GenEventBarcodes> ("barcodes");
  if (!barcodes) {
    barcodes = std::make_shared<GenEventBarcodes>();
    e->add_attribute("barcodes", barcodes);
  }
  // force re-parsing as calling barcodes->is_parsed() returns true here
  barcodes->fillAttribute(e);
}

inline ConstGenVertexPtr  barcode_to_vertex(const GenEvent* e, int id ) {
  // Prefer to use optimized GenEvent barcodes attribute
  const auto &barcodes = e->attribute<GenEventBarcodes> ("barcodes");
  if (barcodes) {
    ConstGenVertexPtr ptr = barcodes->barcode_to_vertex (id);
    if (ptr) return ptr;
  }
  // Fallback to unoptimized GenVertex barcode attribute
  const auto eventAttributes = e->attributes(); // this makes a copy
  const auto barcodeAttributeIt = eventAttributes.find("barcode");
  const bool hasBarcodeAttribute = barcodeAttributeIt != eventAttributes.end();

  const auto &vertices = e->vertices();
  if (hasBarcodeAttribute) {
    for (size_t i = 1; i <= vertices.size(); i++) {
      const auto &ptrIt = barcodeAttributeIt->second.find(-i);
      if (ptrIt != barcodeAttributeIt->second.end()) {
        const auto &ptr = ptrIt->second;
        if (ptr->is_parsed()) {
          if (id == static_cast<HepMC3::IntAttribute*>(ptr.get())->value()) {
            return ptr->vertex();
          }
        }
        else {
          if (id == std::atoi(ptr->unparsed_string().c_str())) {
            return ptr->vertex();
          }
        }
      }
    }
  }
  // No barcodes attribute, so assume that we are passing the id member variable instead of a barcode
  if (-id > 0 && -id <= static_cast<int>(vertices.size())) {
    if (!vertices[-id-1]->attribute<HepMC3::IntAttribute>("barcode")) {
      return vertices[-id-1];
    }
  }
  return  HepMC3::ConstGenVertexPtr();
}

inline ConstGenParticlePtr  barcode_to_particle(const GenEvent* e, int id ) {
  // Prefer to use optimized GenEvent barcodes attribute
  const auto &barcodes = e->attribute<GenEventBarcodes> ("barcodes");
  if (barcodes) {
    ConstGenParticlePtr ptr = barcodes->barcode_to_particle (id);
    if (ptr) return ptr;
  }
  // Fallback to unoptimized GenParticle barcode attribute
  const auto eventAttributes = e->attributes(); // this makes a copy
  const auto barcodeAttributeIt = eventAttributes.find("barcode");
  const bool hasBarcodeAttribute = barcodeAttributeIt != eventAttributes.end();

  const auto &particles = e->particles();
  if (hasBarcodeAttribute) {
    for (size_t i = 1; i <= particles.size(); i++) {
      const auto &ptrIt = barcodeAttributeIt->second.find(i);
      if (ptrIt != barcodeAttributeIt->second.end()) {
        const auto &ptr = ptrIt->second;
        if (ptr->is_parsed()) {
          if (id == static_cast<HepMC3::IntAttribute*>(ptr.get())->value()) {
            return ptr->particle();
          }
        }
        else {
          if (id == std::atoi(ptr->unparsed_string().c_str())) {
            return ptr->particle();
          }
        }
      }
    }
  }
  // No barcodes attribute, so assume that we are passing the id member variable instead of a barcode
  if (id > 0 && id <= static_cast<int>(particles.size())) {
    if (!particles[id-1]->attribute<HepMC3::IntAttribute>("barcode")) {
      return particles[id-1];
    }
  }
  return  HepMC3::ConstGenParticlePtr();
}

inline GenVertexPtr  barcode_to_vertex(GenEvent* e, int id ) {
  // Prefer to use optimized GenEvent barcodes attribute
  const auto &barcodes = e->attribute<GenEventBarcodes> ("barcodes");
  if (barcodes) {
    GenVertexPtr ptr = barcodes->barcode_to_vertex (id);
    if (ptr) return ptr;
  }
  // Fallback to unoptimized GenVertex barcode attribute
  const auto eventAttributes = e->attributes(); // this makes a copy
  const auto barcodeAttributeIt = eventAttributes.find("barcode");
  const bool hasBarcodeAttribute = barcodeAttributeIt != eventAttributes.end();

  const auto &vertices = e->vertices();
  if (hasBarcodeAttribute) {
    for (size_t i = 1; i <= vertices.size(); i++) {
      const auto &ptrIt = barcodeAttributeIt->second.find(-i);
      if (ptrIt != barcodeAttributeIt->second.end()) {
        const auto &ptr = ptrIt->second;
        if (ptr->is_parsed()) {
          if (id == static_cast<HepMC3::IntAttribute*>(ptr.get())->value()) {
            return ptr->vertex();
          }
        }
        else {
          if (id == std::atoi(ptr->unparsed_string().c_str())) {
            return ptr->vertex();
          }
        }
      }
    }
  }
  // No barcodes attribute, so assume that we are passing the id member variable instead of a barcode
  if (-id > 0 && -id <= static_cast<int>(vertices.size())) {
    if (!vertices[-id-1]->attribute<HepMC3::IntAttribute>("barcode")) {
      return vertices[-id-1];
    }
  }
  return  HepMC3::GenVertexPtr();
}

inline GenParticlePtr  barcode_to_particle(GenEvent* e, int id ) {
  // Prefer to use optimized GenEvent barcodes attribute
  const auto &barcodes = e->attribute<GenEventBarcodes> ("barcodes");
  if (barcodes) {
    GenParticlePtr ptr = barcodes->barcode_to_particle (id);
    if (ptr) return ptr;
  }
  // Fallback to unoptimized GenParticle barcode attribute
  const auto eventAttributes = e->attributes(); // this makes a copy
  const auto barcodeAttributeIt = eventAttributes.find("barcode");
  const bool hasBarcodeAttribute = barcodeAttributeIt != eventAttributes.end();

  const auto &particles = e->particles();
  if (hasBarcodeAttribute) {
    for (size_t i = 1; i <= particles.size(); i++) {
      const auto &ptrIt = barcodeAttributeIt->second.find(i);
      if (ptrIt != barcodeAttributeIt->second.end()) {
        const auto &ptr = ptrIt->second;
        if (ptr->is_parsed()) {
          if (id == static_cast<HepMC3::IntAttribute*>(ptr.get())->value()) {
            return ptr->particle();
          }
        }
        else {
          if (id == std::atoi(ptr->unparsed_string().c_str())) {
            return ptr->particle();
          }
        }
      }
    }
  }
  // No barcodes attribute, so assume that we are passing the id member variable instead of a barcode
  if (id > 0 && id <= static_cast<int>(particles.size())) {
    if (!particles[id-1]->attribute<HepMC3::IntAttribute>("barcode")) {
      return particles[id-1];
    }
  }
  return  HepMC3::GenParticlePtr();
}

inline int mpi(const GenEvent& evt) {
    std::shared_ptr<HepMC3::IntAttribute> A_mpi=evt.attribute<HepMC3::IntAttribute>("mpi");
    return A_mpi?(A_mpi->value()):0;
}
inline int mpi(const GenEvent* evt) {
    std::shared_ptr<HepMC3::IntAttribute> A_mpi=evt->attribute<HepMC3::IntAttribute>("mpi");
    return A_mpi?(A_mpi->value()):0;
}

inline int signal_process_id(const GenEvent& evt) {
    std::shared_ptr<HepMC3::IntAttribute> A_signal_process_id=evt.attribute<HepMC3::IntAttribute>("signal_process_id");
    return A_signal_process_id?(A_signal_process_id->value()):0;
}
inline int signal_process_id(const GenEvent* evt) {
    std::shared_ptr<HepMC3::IntAttribute> A_signal_process_id=evt->attribute<HepMC3::IntAttribute>("signal_process_id");
    return A_signal_process_id?(A_signal_process_id->value()):0;
}
inline void set_signal_process_id(GenEvent* e, const int i=0) {
    std::shared_ptr<HepMC3::IntAttribute> signal_process_id = std::make_shared<HepMC3::IntAttribute>(i);
    e->add_attribute("signal_process_id",std::move(signal_process_id));
}
inline void set_mpi(GenEvent* e, const int i=0) {
    std::shared_ptr<HepMC3::IntAttribute> mpi = std::make_shared<HepMC3::IntAttribute>(i);
    e->add_attribute("mpi",std::move(mpi));
}
inline void set_random_states(GenEvent* e, std::vector<long int>& a) {
    e->add_attribute("random_states",std::make_shared<HepMC3::VectorLongIntAttribute>(a));
}
template <class T> void set_signal_process_vertex(GenEvent* e, T& v) {
    if (!v || !e) return;
/* AV: HepMC2 adds the vertex to event */
    e->add_vertex(v);
    v->add_attribute("signal_process_vertex",std::make_shared<HepMC3::IntAttribute>(1));
}
inline ConstGenVertexPtr signal_process_vertex(const GenEvent* e) { for (auto v: e->vertices()) if (v->attribute<HepMC3::IntAttribute>("signal_process_vertex")) return v; return nullptr; }
inline      GenVertexPtr signal_process_vertex(GenEvent* e) { for (auto v: e->vertices()) if (v->attribute<HepMC3::IntAttribute>("signal_process_vertex")) return v; return nullptr; }
inline bool valid_beam_particles(const GenEvent* e) { 
  if (!e) return false; 
  size_t nBeams = 0;
  for (const auto& p : e->beams()) { if (p->status() == 4)  ++nBeams; }
  if  (nBeams != 2) return false; 
  return true;
}

template <class T> bool suggest_barcode(T& p, int i) {
  if (!p->parent_event()) return false;
  auto barcodes = p->parent_event()->template attribute<GenEventBarcodes> ("barcodes");
  if (!barcodes) {
    barcodes = std::make_shared<GenEventBarcodes>();
    p->parent_event()->add_attribute("barcodes", barcodes);
  }
  barcodes->remove(p);
  bool ret = p->add_attribute("barcode",std::make_shared<HepMC3::IntAttribute>(i));
  if (ret) barcodes->add(p);
  return ret;
}

}

#else

#include "HepMC/GenEvent.h"
#include "HepMC/GenVertex.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/Barcode.h"
#include <memory>
namespace HepMC {
inline bool set_ll_event_number(HepMC::GenEvent* e, long long int num){
  if (num > std::numeric_limits<int>::max()) return false;
  e->set_event_number((int)num);
  return true;
}
inline long long int get_ll_event_number(const HepMC::GenEvent* e){
  return e->event_number();
}
inline GenEvent::particle_iterator  begin(HepMC::GenEvent& e) { return e.particles_begin(); }
inline GenEvent::particle_iterator  end(HepMC::GenEvent& e) { return e.particles_end(); }
inline GenEvent::particle_const_iterator  begin(const HepMC::GenEvent& e) { return e.particles_begin(); }
inline GenEvent::particle_const_iterator  end(const HepMC::GenEvent& e) { return e.particles_end(); }
inline GenEvent* newGenEvent(const int a, const int b ) { return new GenEvent(a,b); }
inline GenVertex* signal_process_vertex(const GenEvent* e) { return e->signal_process_vertex(); }
inline void fillBarcodesAttribute(GenEvent* ) { }
inline GenVertex* barcode_to_vertex(const GenEvent* e, int id ) {return  e->barcode_to_vertex(id);}
inline GenParticle* barcode_to_particle(const GenEvent* e, int id ) {return  e->barcode_to_particle(id);}
inline int mpi(const GenEvent& e) {
    return e.mpi();
}
inline int mpi(const GenEvent* e) {
    return e->mpi();
}
inline int signal_process_id(const GenEvent& e) {
    return e.signal_process_id();
}
inline int signal_process_id(const GenEvent* e) {
    return e->signal_process_id();
}
inline void set_signal_process_id(GenEvent* e, const int i) {
    e->set_signal_process_id(i);
}
inline void set_mpi(GenEvent* e, const int i) {
    e->set_mpi(i);
}
template <class T> void set_random_states(GenEvent* e, std::vector<T> a) {
    e->set_random_states(a);
}
template <class T> void set_signal_process_vertex(GenEvent* e, T v) {
    e->set_signal_process_vertex(v);
}
inline GenEvent* copyemptyGenEvent(const GenEvent* inEvt) {
    HepMC::GenEvent* outEvt = new HepMC::GenEvent( inEvt->signal_process_id(),  inEvt->event_number() );
    outEvt->set_mpi  ( inEvt->mpi() );
    outEvt->set_event_scale  ( inEvt->event_scale() );
    outEvt->set_alphaQCD     ( inEvt->alphaQCD() );
    outEvt->set_alphaQED     ( inEvt->alphaQED() );
    outEvt->weights() =        inEvt->weights();
    outEvt->set_random_states( inEvt->random_states() );
    if ( nullptr != inEvt->heavy_ion() ) {
      outEvt->set_heavy_ion    ( *inEvt->heavy_ion() );
    }
    if ( nullptr != inEvt->pdf_info() ) {
      outEvt->set_pdf_info     ( *inEvt->pdf_info() );
    }
    return outEvt;
}

template <class T> bool suggest_barcode(T& p, int i) {return p.suggest_barcode(i);}
template <class T> bool suggest_barcode(T* p, int i) {return p->suggest_barcode(i);}
//Smart pointers should not be used with HepMC2. But it happens.
template <> inline  bool suggest_barcode<std::unique_ptr<HepMC::GenParticle> >(std::unique_ptr<HepMC::GenParticle>& p, int i) {return p->suggest_barcode(i);}

namespace Print {
inline void line(std::ostream& os,const GenEvent& e) {e.print(os);}
inline void line(std::ostream& os,const GenEvent* e) {e->print(os);}
inline void content(std::ostream& os,const GenEvent& e) {e.print(os);}
inline void content(std::ostream& os,const GenEvent* e) {e->print(os);}
}
inline bool valid_beam_particles(const GenEvent* e) {return e->valid_beam_particles();}
}
#include "AtlasHepMC/SimpleVector.h"
#endif
#endif
