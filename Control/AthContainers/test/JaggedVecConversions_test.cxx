/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/JaggedVecConversions_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2024
 * @brief Regression tests for JaggedVec converter classes.
 */


#undef NDEBUG
#include "AthContainers/tools/JaggedVecConversions.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "TestTools/expect_exception.h"
#include <vector>
#include <ranges>
#include <functional>
#include <iostream>
#include <cassert>


namespace {


template <class T>
void compareElts (const SG::JaggedVecElt<T>* ptr, size_t n,
                  const std::vector<SG::JaggedVecElt<T> >& v)
{
  assert (n >= v.size());
  for (size_t i = 0; i  < v.size(); ++i) {
    if (ptr[i] != v[i]) {
      std::cerr << "Elt comparison failure: [";
      for (size_t ii = 0; ii  < v.size(); ++ii)
        std::cerr << ptr[ii].end() << " ";
      std::cerr << "] [";
      for (size_t ii = 0; ii  < v.size(); ++ii)
        std::cerr << v[ii].end() << " ";
      std::cerr << "]\n";
      std::abort();
    }
  }
  SG::JaggedVecElt<T> tail (v.back().end());
  for (size_t i = v.size(); i  < n; ++i) {
    assert (ptr[i] == tail);
  }
}


template <class T>
void comparePayload (const T* ptr, const std::vector<T>& v)
{
  if (!std::equal (v.begin(), v.end(), ptr, std::equal_to<T>())) {
    std::cerr << "Payload comparison failure: [";
    for (size_t ii = 0; ii  < v.size(); ++ii)
      std::cerr << ptr[ii] << " ";
    std::cerr << "] [";
    for (size_t ii = 0; ii  < v.size(); ++ii)
      std::cerr << v[ii] << " ";
    std::cerr << "]\n";
    std::abort();
  }
}


} // anonymous namespace

namespace SG {


class AuxVectorBase
  : public SG::AuxVectorData
{
public:
  AuxVectorBase (size_t sz = 10) : m_sz (sz) {}
  virtual size_t size_v() const override { return m_sz; }
  virtual size_t capacity_v() const override { return m_sz; }

  using SG::AuxVectorData::setStore;

private:
  size_t m_sz;
};


}


void test_JaggedVecConstConverter()
{
  std::cout << "test_JaggedVecConstConverter\n";

  std::vector<int> v1 { 1, 2, 3, 4, 5 };
  SG::AuxDataSpanBase sp1 { v1.data(), v1.size() };
  SG::JaggedVecEltBase elts[] = {{2}, {4}, {2}, {10}};
  SG::detail::JaggedVecConstConverter<int> c1 (elts, sp1);
  auto r1 = c1 (elts[1]);
  assert (r1.size() == 2);
  assert (r1[1] == 4);
  auto r2 = c1 (elts[0]);
  assert (r2.size() == 2);
  assert (r2[1] == 2);

  EXPECT_EXCEPTION( std::out_of_range, c1 (elts[2]) );
  EXPECT_EXCEPTION( std::out_of_range, c1 (elts[3]) );
}


void test_JaggedVecProxyBase()
{
  std::cout << "test_JaggedVecProxyBase\n";

  using Payload_t = int;
  using Elt_t = SG::JaggedVecElt<int>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t payload_id = r.getAuxID<Payload_t>
    ("jvec_linked", "", SG::AuxVarFlags::Linked);
  SG::auxid_t jvec_id = r.getAuxID<Elt_t>
    ("jvec", "", SG::AuxVarFlags::None, payload_id);

  SG::AuxVectorBase v;
  SG::AuxStoreInternal store;
  v.setStore (&store);

  Elt_t* elt = reinterpret_cast<Elt_t*> (store.getData(jvec_id, 10, 10));
  Payload_t* payload = reinterpret_cast<Payload_t*> (store.getData(payload_id, 5, 5));

  std::ranges::copy (std::vector<Elt_t>{{2}, {2}, {5}}, elt);
  std::fill_n (elt+3, 7, Elt_t{5});
  std::ranges::copy (std::vector<Payload_t> {1, 2, 3, 4, 5}, payload);
  SG::AuxDataSpanBase elt_span { elt, 10 };
  SG::IAuxTypeVector* linkedVec = store.linkedVector (jvec_id);

  SG::detail::JaggedVecProxyBase pb (elt_span, v, jvec_id);
  assert (pb.elt(2) == Elt_t(5));
  const SG::detail::JaggedVecProxyBase& cpb = pb;
  assert (cpb.elt(2) == Elt_t(5));

  pb.adjust1 (0, 1, 1);
  compareElts (elt, 10, {{3}, {3}, {6}});
  assert (linkedVec->size() == 6);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  comparePayload (payload, {1, 0, 2, 3, 4, 5});

  pb.adjust1 (2, 2, -2);
  compareElts (elt, 10, {{3}, {3}, {4}});
  assert (linkedVec->size() == 4);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  comparePayload (payload, {1, 0, 2, 5});

  pb.resize1 (1, 2);
  compareElts (elt, 10, {{3}, {5}, {6}});
  assert (linkedVec->size() == 6);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  comparePayload (payload, {1, 0, 2, 0, 0, 5});

  pb.resize1 (0, 1);
  compareElts (elt, 10, {{1}, {3}, {4}});
  assert (linkedVec->size() == 4);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  comparePayload (payload, {1, 0, 0, 5});
}


void test_JaggedVecProxyValBase()
{
  std::cout << "test_JaggedVecProxyValBase\n";

  using Payload_t = int;
  using Elt_t = SG::JaggedVecElt<int>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t payload_id = r.getAuxID<Payload_t>
    ("jvec_linked", "", SG::AuxVarFlags::Linked);
  SG::auxid_t jvec_id = r.getAuxID<Elt_t>
    ("jvec", "", SG::AuxVarFlags::None, payload_id);

  SG::AuxVectorBase v;
  SG::AuxStoreInternal store;
  v.setStore (&store);

  Elt_t* elt = reinterpret_cast<Elt_t*> (store.getData(jvec_id, 10, 10));
  Payload_t* payload = reinterpret_cast<Payload_t*> (store.getData(payload_id, 5, 5));

  std::ranges::copy (std::vector<Elt_t>{{2}, {2}, {5}}, elt);
  std::fill_n (elt+3, 7, Elt_t{5});
  std::ranges::copy (std::vector<Payload_t> {1, 2, 3, 4, 5}, payload);
  SG::AuxDataSpanBase elt_span { elt, 10 };

  SG::detail::JaggedVecProxyValBase pb (elt_span, v, jvec_id);
  assert (pb.m_base.elt(2) == Elt_t(5));
}


void test_JaggedVecProxyRefBase()
{
  std::cout << "test_JaggedVecProxyRefBase\n";

  using Payload_t = int;
  using Elt_t = SG::JaggedVecElt<int>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t payload_id = r.getAuxID<Payload_t>
    ("jvec_linked", "", SG::AuxVarFlags::Linked);
  SG::auxid_t jvec_id = r.getAuxID<Elt_t>
    ("jvec", "", SG::AuxVarFlags::None, payload_id);

  SG::AuxVectorBase v;
  SG::AuxStoreInternal store;
  v.setStore (&store);

  Elt_t* elt = reinterpret_cast<Elt_t*> (store.getData(jvec_id, 10, 10));
  Payload_t* payload = reinterpret_cast<Payload_t*> (store.getData(payload_id, 5, 5));

  std::ranges::copy (std::vector<Elt_t>{{2}, {2}, {5}}, elt);
  std::fill_n (elt+3, 7, Elt_t{5});
  std::ranges::copy (std::vector<Payload_t> {1, 2, 3, 4, 5}, payload);
  SG::AuxDataSpanBase elt_span { elt, 10 };

  SG::detail::JaggedVecProxyBase pb (elt_span, v, jvec_id);
  SG::detail::JaggedVecProxyRefBase prb (pb);
  assert (prb.m_base.elt(2) == Elt_t(5));
}


void test_JaggedVecProxy()
{
  std::cout << "test_JaggedVecProxy\n";

  using Payload_t = int;
  using Elt_t = SG::JaggedVecElt<int>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t payload_id = r.getAuxID<Payload_t>
    ("jvec_linked", "", SG::AuxVarFlags::Linked);
  SG::auxid_t jvec_id = r.getAuxID<Elt_t>
    ("jvec", "", SG::AuxVarFlags::None, payload_id);

  SG::AuxVectorBase v;
  SG::AuxStoreInternal store;
  v.setStore (&store);

  Elt_t* elt = reinterpret_cast<Elt_t*> (store.getData(jvec_id, 10, 10));
  Payload_t* payload = reinterpret_cast<Payload_t*> (store.getData(payload_id, 5, 5));

  std::ranges::copy (std::vector<Elt_t>{{2}, {2}, {5}}, elt);
  std::fill_n (elt+3, 7, Elt_t{5});
  std::ranges::copy (std::vector<Payload_t> {1, 2, 3, 4, 5}, payload);
  SG::AuxDataSpanBase elt_span { elt, 10 };
  SG::IAuxTypeVector* linkedVec = store.linkedVector (jvec_id);

  using ValProxy_t = SG::detail::JaggedVecProxyT<int, SG::detail::JaggedVecProxyValBase>;
  using RefProxy_t = SG::detail::JaggedVecProxyT<int, SG::detail::JaggedVecProxyRefBase>;

  ValProxy_t p0 (0, linkedVec->getDataSpan(), elt_span, v, jvec_id);
  const ValProxy_t& cp0 = p0;

  assert (p0.size() == 2);
  assert (!p0.empty());

  SG::detail::JaggedVecProxyBase pb (elt_span, v, jvec_id);
  RefProxy_t p1 (1, linkedVec->getDataSpan(), pb);
  RefProxy_t p2 (2, linkedVec->getDataSpan(), pb);
  const RefProxy_t& cp2 = p2;

  assert (p1.size() == 0);
  assert (p1.empty());

  assert (p2.size() == 3);
  assert (!p2.empty());

  assert (p0[1] == 2);
  assert (cp0[0] == 1);

  assert (p2.at(1) == 4);
  assert (cp2.at(0) == 3);
  EXPECT_EXCEPTION( std::out_of_range, p2.at(4) );
  EXPECT_EXCEPTION( std::out_of_range, cp2.at(4) );

  p0[0] = 6;
  p2.at(0) = 7;

  assert (p0.front() == 6);
  assert (p0.back() == 2);
  assert (cp2.front() == 7);
  assert (cp2.back() == 5);

  {
    std::vector<int> v (p0.begin(), p0.end());
    assert (v == (std::vector<int> {6, 2}));
  }
  {
    std::vector<int> v (p0.cbegin(), p0.cend());
    assert (v == (std::vector<int> {6, 2}));
  }
  {
    std::vector<int> v (cp2.begin(), cp2.end());
    assert (v == (std::vector<int> {7, 4, 5}));
  }
  {
    std::vector<int> v (p0.rbegin(), p0.rend());
    assert (v == (std::vector<int> {2, 6}));
  }
  {
    std::vector<int> v (p0.crbegin(), p0.crend());
    assert (v == (std::vector<int> {2, 6}));
  }
  {
    std::vector<int> v (cp2.rbegin(), cp2.rend());
    assert (v == (std::vector<int> {5, 4, 7}));
  }

  for (int& i : p0) {
    i += 10;
  }
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 5);
  comparePayload (payload, {16, 12, 7, 4, 5});

  p0 = std::vector<int> {8, 9, 10};
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 6);
  compareElts (elt, 10, {{3}, {3}, {6}});
  comparePayload (payload, {8, 9, 10, 7, 4, 5});

  assert (p0.asVector() == (std::vector<int> {8, 9, 10}));
  {
    std::vector<int> v = p2;
    assert (v == (std::vector<int> {7, 4, 5}));
  }

  p1.push_back (12);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 7);
  compareElts (elt, 10, {{3}, {4}, {7}});
  comparePayload (payload, {8, 9, 10, 12, 7, 4, 5});

  p2.clear();
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 4);
  compareElts (elt, 10, {{3}, {4}, {4}});
  comparePayload (payload, {8, 9, 10, 12});

  p1.resize (3, 16);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 6);
  compareElts (elt, 10, {{3}, {6}, {6}});
  comparePayload (payload, {8, 9, 10, 12, 16, 16});

  p1.resize (2);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 5);
  compareElts (elt, 10, {{3}, {5}, {5}});
  comparePayload (payload, {8, 9, 10, 12, 16});

  p0.erase (p0.begin()+1);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 4);
  compareElts (elt, 10, {{2}, {4}, {4}});
  comparePayload (payload, {8, 10, 12, 16});

  p1.insert (p1.begin()+1, 17);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 5);
  compareElts (elt, 10, {{2}, {5}, {5}});
  comparePayload (payload, {8, 10, 12, 17, 16});

  p1.erase (p1.begin(), p1.begin()+2);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 3);
  compareElts (elt, 10, {{2}, {3}, {3}});
  comparePayload (payload, {8, 10, 16});

  p2.assign (3, 18);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 6);
  compareElts (elt, 10, {{2}, {3}, {6}});
  comparePayload (payload, {8, 10, 16, 18, 18, 18});

  p2.insert (p2.begin()+1, 2, 19);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 8);
  compareElts (elt, 10, {{2}, {3}, {8}});
  comparePayload (payload, {8, 10, 16, 18, 19, 19, 18, 18});

  {
    std::vector<int> v {20, 21};
    p0.insert (p0.begin(), v.begin(), v.end());
  }
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 10);
  compareElts (elt, 10, {{4}, {5}, {10}});
  comparePayload (payload, {20, 21, 8, 10, 16, 18, 19, 19, 18, 18});

  p1.insert_range (p1.begin(), std::vector<int> {22});
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 11);
  compareElts (elt, 10, {{4}, {6}, {11}});
  comparePayload (payload, {20, 21, 8, 10, 22, 16, 18, 19, 19, 18, 18});

  p1.append_range (std::vector<int> {23, 24});
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 13);
  compareElts (elt, 10, {{4}, {8}, {13}});
  comparePayload (payload, {20, 21, 8, 10, 22, 16, 23, 24, 18, 19, 19, 18, 18});

  p2.pop_back();
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 12);
  compareElts (elt, 10, {{4}, {8}, {12}});
  comparePayload (payload, {20, 21, 8, 10, 22, 16, 23, 24, 18, 19, 19, 18});

  {
    std::vector<int> v {25, 26};
    p0.assign (v.begin(), v.end());
  }
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 10);
  compareElts (elt, 10, {{2}, {6}, {10}});
  comparePayload (payload, {25, 26, 22, 16, 23, 24, 18, 19, 19, 18});

  p2.assign_range (std::vector<int> {27, 28});
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 8);
  compareElts (elt, 10, {{2}, {6}, {8}});
  comparePayload (payload, {25, 26, 22, 16, 23, 24, 27, 28});

  p1.erase (2);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 7);
  compareElts (elt, 10, {{2}, {5}, {7}});
  comparePayload (payload, {25, 26, 22, 16, 24, 27, 28});

  p0.insert (1, 29);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 8);
  compareElts (elt, 10, {{3}, {6}, {8}});
  comparePayload (payload, {25, 29, 26, 22, 16, 24, 27, 28});

  p1.erase (0, 2);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 6);
  compareElts (elt, 10, {{3}, {4}, {6}});
  comparePayload (payload, {25, 29, 26, 24, 27, 28});

  p1.insert (0, 2, 30);
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 8);
  compareElts (elt, 10, {{3}, {6}, {8}});
  comparePayload (payload, {25, 29, 26, 30, 30, 24, 27, 28});

  {
    std::vector<int> v {31, 32};
    p2.insert (1, v.begin(), v.end());
  }
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 10);
  compareElts (elt, 10, {{3}, {6}, {10}});
  comparePayload (payload, {25, 29, 26, 30, 30, 24, 27, 31, 32, 28});

  p0.insert_range (1, std::vector {33, 34});
  payload = reinterpret_cast<Payload_t*> (linkedVec->toPtr());
  assert (linkedVec->size() == 12);
  compareElts (elt, 10, {{5}, {8}, {12}});
  comparePayload (payload, {25, 33, 34, 29, 26, 30, 30, 24, 27, 31, 32, 28});
}


void test_JaggedVecConverter()
{
  std::cout << "test_JaggedVecConverter\n";

  using Payload_t = int;
  using Elt_t = SG::JaggedVecElt<int>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t payload_id = r.getAuxID<Payload_t>
    ("jvec_linked", "", SG::AuxVarFlags::Linked);
  SG::auxid_t jvec_id = r.getAuxID<Elt_t>
    ("jvec", "", SG::AuxVarFlags::None, payload_id);

  SG::AuxVectorBase v;
  SG::AuxStoreInternal store;
  v.setStore (&store);

  Elt_t* elt = reinterpret_cast<Elt_t*> (store.getData(jvec_id, 10, 10));
  const Elt_t* elt_c = elt;
  Payload_t* payload = reinterpret_cast<Payload_t*> (store.getData(payload_id, 5, 5));

  std::ranges::copy (std::vector<Elt_t>{{2}, {2}, {5}}, elt);
  std::fill_n (elt+3, 7, Elt_t{5});
  std::ranges::copy (std::vector<Payload_t> {1, 2, 3, 4, 5}, payload);
  SG::AuxDataSpanBase elt_span { elt, 10 };

  SG::detail::JaggedVecConverter<Payload_t> c1 (v, jvec_id, payload_id);
  auto r1 = c1 (elt_c[2]);
  assert (r1.size() == 3);
  assert (r1[1] == 4);

  auto r2 = c1 (elt[0]);
  assert (r2.size() == 2);
  assert (r2[0] == 1);

  r2[1] = 10;
  assert (payload[1]== 10);
}

//coverity[root_function]
int main()
{
  std::cout << "AthContainers/JaggedVecConversions_test\n";
  test_JaggedVecConstConverter();
  test_JaggedVecProxyBase();
  test_JaggedVecProxyValBase();
  test_JaggedVecProxyRefBase();
  test_JaggedVecProxy();
  test_JaggedVecConverter();
  return 0;
}
