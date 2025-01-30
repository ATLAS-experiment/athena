/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file  src/xAODTestTypelessRead.cxx
 * @author snyder@bnl.gov
 * @date May 2014
 * @brief Algorithm to test reading @c DataVector with auxiliary data,
 *        without compile-time typing of aux data.
 */


#include "xAODTestTypelessRead.h"
#include "DataModelTestDataCommon/C.h"
#include "DataModelTestDataCommon/CVec.h"
#include "DataModelTestDataCommon/CVecWithData.h"
#include "DataModelTestDataCommon/CView.h"
#include "DataModelTestDataCommon/CAuxContainer.h"
#include "DataModelTestDataCommon/CTrigAuxContainer.h"
#include "DataModelTestDataCommon/CInfoAuxContainer.h"
#include "DataModelTestDataCommon/P.h"
#include "DataModelTestDataCommon/PVec.h"
#include "DataModelTestDataCommon/PAuxContainer.h"
#include "DataModelTestDataRead/H.h"
#include "DataModelTestDataRead/HAuxContainer.h"
#include "DataModelTestDataRead/HVec.h"
#include "DataModelTestDataRead/HView.h"
#include "DataModelTestDataCommon/JVecContainer.h"
#include "DataModelTestDataCommon/JVec.h"
#include "DataModelTestDataCommon/JVecAuxContainer.h"
#include "DataModelTestDataCommon/JVecAuxInfo.h"
#include "DataModelTestDataCommon/PLinksContainer.h"
#include "DataModelTestDataCommon/PLinks.h"
#include "DataModelTestDataCommon/PLinksAuxContainer.h"
#include "DataModelTestDataCommon/PLinksAuxInfo.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/JaggedVec.h"
#include "AthLinks/ElementLink.h"
#include "AthenaKernel/errorcheck.h"
#include "CxxUtils/StrFormat.h"
#include "GaudiKernel/System.h"
#include <memory>
#include <sstream>


namespace DMTest {


namespace {


template <class CONT>
std::string formEL (const ElementLink<CONT>& el)
{
  std::string index = el.isDefaultIndex() ?
    "inv" :
    std::to_string (el.index());

  return el.dataID() + "[" + index + "]";
}


template <class T>
std::string form_vec_elt (const T& x)
{
  std::ostringstream ss;
  ss << x;
  return ss.str();
}
  


std::string form_vec_elt (const std::string& x)
{
  return "'" + x + "'";
}
  

template <class CONT>
std::string form_vec_elt (const ElementLink<CONT>& x)
{
  return formEL (x);
}
  

template <class RANGE>
std::string formJVec (const RANGE& r) {
  std::ostringstream ss;
  ss << "[";
  std::string sep;
  for (const auto & elt : r) {
    ss << sep;
    sep = " ";
    ss << form_vec_elt (elt);
  }
  ss << "]";
  return ss.str();
}


void dumpAuxItem (std::ostream& ost,
                  SG::auxid_t auxid,
                  const SG::AuxVectorData& c, size_t i)
{
  const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  if (r.isLinked (auxid)) return;
  const std::type_info* ti = r.getType(auxid);
  ost << "\n    " << r.getName(auxid) << ": ";
  if (ti == &typeid(int))
    ost << c.getData<int> (auxid, i) << "; ";
  else if (ti == &typeid(unsigned int))
    ost << c.getData<unsigned int> (auxid, i) << "; ";
  else if (ti == &typeid(float))
    ost << CxxUtils::strformat ("%.3f", c.getData<float> (auxid, i)) << "; ";
  else if (ti == &typeid(ElementLink<DMTest::CVec>)) {
    const ElementLink<DMTest::CVec>& el =
      c.getData<ElementLink<DMTest::CVec> > (auxid, i);
    ost << formEL (el) << "; ";
  }
#if 0
  else if (ti == &typeid(SG::PackedElement<unsigned int>))
    ost << c.getData<SG::PackedElement<unsigned int> > (auxid, i) << "; ";
  else if (ti == &typeid(SG::PackedElement<float>))
    ost << c.getData<SG::PackedElement<float> > (auxid, i) << "; ";
#endif
  else if (ti == &typeid(std::vector<unsigned int>)) {
    ost << "[";
    for (auto ii : c.getData<std::vector<unsigned int> > (auxid, i))
      ost << ii << " ";
    ost << "]; ";
  }
  else if (ti == &typeid(std::vector<int>)) {
    ost << "[";
    for (auto ii : c.getData<std::vector<int> > (auxid, i))
      ost << ii << " ";
    ost << "]; ";
  }
  else if (ti == &typeid(std::vector<float>) ||
           strcmp (ti->name(), typeid(std::vector<float>).name()) == 0)
  {
    ost << "[";
    for (auto ii : c.getData<std::vector<float> > (auxid, i))
      ost << CxxUtils::strformat ("%.3f", ii) << " ";
    ost << "]; ";
  }
#if 0
  else if (ti == &typeid(SG::PackedElement<std::vector<unsigned int> >)) {
    ost << "[";
    for (auto ii : c.getData<SG::PackedElement<std::vector<unsigned int> > > (auxid, i))
      ost << ii << " ";
    ost << "]; ";
  }
  else if (ti == &typeid(SG::PackedElement<std::vector<int> >)) {
    ost << "[";
    for (auto ii : c.getData<SG::PackedElement<std::vector<int> > > (auxid, i))
      ost << ii << " ";
    ost << "]; ";
  }
  else if (ti == &typeid(SG::PackedElement<std::vector<float> >)) {
    ost << "[";
    for (auto ii : c.getData<SG::PackedElement<std::vector<float> > > (auxid, i))
      ost << CxxUtils::strformat ("%.3f", ii) << " ";
    ost << "]; ";
  }
#endif
  else if (ti == &typeid(SG::JaggedVecElt<int>)) {
    SG::ConstAccessor<SG::JaggedVecElt<int> > acc (r.getName (auxid));
    ost << formJVec (acc (c, i)) << "; ";
  }
  else if (ti == &typeid(SG::JaggedVecElt<float>)) {
    SG::ConstAccessor<SG::JaggedVecElt<float> > acc (r.getName (auxid));
    ost << formJVec (acc (c, i)) << "; ";
  }
  else if (ti == &typeid(SG::JaggedVecElt<double>)) {
    SG::ConstAccessor<SG::JaggedVecElt<double> > acc (r.getName (auxid));
    ost << formJVec (acc (c, i)) << "; ";
  }
  else if (ti == &typeid(SG::JaggedVecElt<std::string>)) {
    SG::ConstAccessor<SG::JaggedVecElt<std::string> > acc (r.getName (auxid));
    ost << formJVec (acc (c, i)) << "; ";
  }
  else if (ti == &typeid(SG::JaggedVecElt<ElementLink<CVec> >)) {
    SG::ConstAccessor<SG::JaggedVecElt<ElementLink<CVec> > > acc (r.getName (auxid));
    ost << formJVec (acc (c, i)) << "; ";
  }
  else if (ti == &typeid(SG::PackedLink<CVec>)) {
    SG::ConstAccessor<SG::PackedLink<CVec> > acc (r.getName (auxid));
    ElementLink<CVec> el = acc (c, i);
    ost << formEL (el) << "; ";
  }
  else if (ti == &typeid(std::vector<SG::PackedLink<CVec> >)) {
    SG::ConstAccessor<std::vector<SG::PackedLink<CVec> > > acc (r.getName (auxid));
    ost << "[";
    for (ElementLink<CVec> el : acc (c, i)) {
      ost << formEL (el) << "; ";
    }
    ost << "]; ";
  }
  else
    ost << "xxx " << ti->name() << "; ";
}


std::map<std::string, SG::auxid_t> get_map (const SG::AuxVectorData* vec)
{
  const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();

  // Sort auxids in name order.
  std::map<std::string, SG::auxid_t> auxid_map;
  for (SG::auxid_t auxid : vec->getAuxIDs())
    auxid_map[r.getName(auxid)] = auxid;

  return auxid_map;
}


std::map<std::string, SG::auxid_t> get_map (const SG::AuxElement* elt)
{
  return get_map (elt->container());
}


void dumpelt (std::ostream& ost,
              const SG::AuxVectorData* cont,
              size_t index,
              const std::map<std::string, SG::auxid_t>& auxid_map)
{
  for (const auto& m : auxid_map)
    dumpAuxItem (ost, m.second, *cont, index);
  ost << "\n";
}


template <class OBJ>
void dumpobj (std::ostream& ost,
              const OBJ* obj,
              const std::map<std::string, SG::auxid_t>& auxid_map)
{
  for (size_t i = 0; i < obj->size(); i++) {
    ost << "  ";
    if (!obj->hasStore()) {
      // Handle view container.
      const auto* elt = obj->at(i);
      dumpelt (ost, elt->container(), elt->index(), auxid_map);
    }
    else
      dumpelt (ost, obj, i, auxid_map);
  }
}


void dumpobj (std::ostream& ost,
              const DMTest::C* obj,
              const std::map<std::string, SG::auxid_t>& auxid_map)
{
  const SG::AuxVectorData* cont = obj->container();
  dumpelt (ost, cont, 0, auxid_map);
}


void dumpobj (std::ostream& ost,
              const DMTest::PLinks* obj,
              const std::map<std::string, SG::auxid_t>& auxid_map)
{
  const SG::AuxVectorData* cont = obj->container();
  dumpelt (ost, cont, 0, auxid_map);
}


void dumpobj (std::ostream& ost,
              const DMTest::JVec* obj,
              const std::map<std::string, SG::auxid_t>& auxid_map)
{
  const SG::AuxVectorData* cont = obj->container();
  dumpelt (ost, cont, 0, auxid_map);
}


template <class CONTAINER>
void copy (CONTAINER& to, const CONTAINER& from)
{
  for (size_t i = 0; i < from.size(); i++) {
    to.push_back (new typename CONTAINER::base_value_type);
    *to.back() = *from[i];
  }
}


void copy (DMTest::CVecWithData& to, const DMTest::CVecWithData& from)
{
  to.meta1 = from.meta1;
  copy (static_cast<DMTest::CVec&>(to),
        static_cast<const DMTest::CVec&>(from));
}


void copy (DMTest::C& to, const DMTest::C& from)
{
  to = from;
}


void copy (DMTest::JVec& to, const DMTest::JVec& from)
{
  to = from;
}


void copy (DMTest::PLinks& to, const DMTest::PLinks& from)
{
  to = from;
}


} // anonymous namespace


template <class OBJ, class AUX>
StatusCode
xAODTestTypelessRead::testit (const std::string& key)
{
  const OBJ* obj = nullptr;
  CHECK( evtStore()->retrieve (obj, key) );

  const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();

  std::map<std::string, SG::auxid_t> auxid_map = get_map (obj);
  std::ostringstream ost1;
  ost1 << key << " types: ";
  for (const auto& m : auxid_map) {
    if (!r.isLinked (m.second)) {
      ost1 << r.getName(m.second) << "/" 
           << System::typeinfoName (*r.getType(m.second)) << " ";
    }
  }
  ATH_MSG_INFO (ost1.str());

  std::ostringstream ost2;
  dumpobj (ost2, obj, auxid_map);
  ATH_MSG_INFO (ost2.str());

  if (!m_writePrefix.empty()) {
    // Passing this as the third arg of record will make the object const.
    bool LOCKED = false;

    auto objnew = std::make_unique<OBJ>();
    auto store  = std::make_unique<AUX>();
    objnew->setStore (store.get());
    copy (*objnew, *obj);
    CHECK (evtStore()->record (std::move(objnew), m_writePrefix + key, LOCKED));
    CHECK (evtStore()->record (std::move(store), m_writePrefix + key + "Aux.", LOCKED));
  }
  return StatusCode::SUCCESS;
}


template <class OBJ>
StatusCode
xAODTestTypelessRead::testit_view (const std::string& key)
{
  const OBJ* obj = nullptr;
  CHECK( evtStore()->retrieve (obj, key) );

  if (!obj->empty()) {
    std::map<std::string, SG::auxid_t> auxid_map = get_map (obj->front());
    ATH_MSG_INFO (key);
    std::ostringstream ost;
    dumpobj (ost, obj, auxid_map);
    ATH_MSG_INFO (ost.str());
  }

  if (!m_writePrefix.empty()) {
    CHECK (evtStore()->record (std::make_unique<OBJ>(*obj),
                               m_writePrefix + key, false));
  }
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestTypelessRead::execute()
{
  ++m_count;
  ATH_MSG_INFO (m_count);

  CHECK(( testit<CVec, CAuxContainer>     ("cvec") ));
  CHECK(( testit<C,    CInfoAuxContainer> ("cinfo") ));
  CHECK(( testit<CVec, CTrigAuxContainer> ("ctrig") ));
  CHECK(( testit<CVecWithData, CAuxContainer> ("cvecWD") ));
  if (!m_cviewKey.empty()) {
    CHECK(( testit_view<CView> (m_cviewKey) ));
  }
  if (!m_pvecKey.empty()) {
    CHECK(( testit<PVec, PAuxContainer>     (m_pvecKey) ));
  }
  CHECK(( testit<HVec, HAuxContainer>     ("hvec") ));
  if (!m_hviewKey.empty()) {
    CHECK(( testit_view<HView> (m_hviewKey) ));
  }
  CHECK(( testit<JVecContainer, JVecAuxContainer> ("jvecContainer") ));
  CHECK(( testit<JVec, JVecAuxInfo> ("jvecInfo") ));
  CHECK(( testit<PLinksContainer, PLinksAuxContainer> ("plinksContainer") ));
  CHECK(( testit<PLinks, PLinksAuxInfo> ("plinksInfo") ));

  return StatusCode::SUCCESS;
}


} // namespace DMTest

