/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/test/ConcurrentToValMap_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2023
 * @brief Tests for ConcurrentToValMap.
 */


#undef NDEBUG
#include "CxxUtils/ConcurrentToValMap.h"
#include "TestTools/expect_exception.h"
#include <algorithm>
#include <shared_mutex>
#include <thread>
#include <vector>
#include <deque>
#include <memory>
#include <mutex>
#include <atomic>
#include <iostream>
#include <cassert>


const int nslots = 4;

struct Context_t
{
  Context_t (int the_slot = 0) : slot (the_slot) {}
  int slot;
};


template <class T>
class TestUpdater
{
public:
  using Context_t = ::Context_t;

  TestUpdater()
    : m_p (nullptr),
      m_inGrace (0)
  {
  }

  TestUpdater (TestUpdater&& other)
    : m_p (static_cast<T*> (other.m_p)),
      m_inGrace (0)
  {
  }

  TestUpdater& operator= (const TestUpdater&) = delete; // coverity

  ~TestUpdater()
  {
    delete m_p;
    for (T* p : m_garbage) delete p;
  }

  void update (std::unique_ptr<T> p, const Context_t& ctx)
  {
    std::lock_guard<std::mutex> g (m_mutex);
    if (m_p) m_garbage.push_back (m_p);
    m_p = p.release();
    m_inGrace = (~(1<<ctx.slot)) & ((1<<nslots)-1);
  }

  void discard (std::unique_ptr<T> p)
  {
    std::lock_guard<std::mutex> g (m_mutex);
    m_garbage.push_back (p.release());
    m_inGrace = ((1<<nslots)-1);
  }

  const T& get() const { return *m_p; }

  void quiescent (const Context_t& ctx)
  {
    unsigned int mask = (1<<ctx.slot);
    std::lock_guard<std::mutex> g (m_mutex);
    if ((m_inGrace & mask) == 0) return;
    m_inGrace &= ~mask;
    if (!m_inGrace) {
      for (T* p : m_garbage) delete p;
      m_garbage.clear();
    }
  }

  static Context_t defaultContext() { return 0; }


  void swap (TestUpdater& other)
  {
    auto swap_atomic = [] (std::atomic<T*>& a, std::atomic<T*>& b)
    {
      T* tmp = a.load (std::memory_order_relaxed);
      a.store (b.load (std::memory_order_relaxed),
               std::memory_order_relaxed);
      b.store (tmp, std::memory_order_relaxed);
    };

    swap_atomic (m_p, other.m_p);
    std::lock_guard<std::mutex> g (m_mutex);
    std::lock_guard<std::mutex> gother (other.m_mutex);
    m_garbage.swap (other.m_garbage);
    std::swap (m_inGrace, other.m_inGrace);
  }


  unsigned int inGrace() const {
    std::lock_guard<std::mutex> g (m_mutex);
    return m_inGrace;
  }


private:
  mutable std::mutex m_mutex;
  std::atomic<T*> m_p;
  std::vector<T*> m_garbage;
  unsigned int m_inGrace;
};


template <class T>
struct Values
{
  Values (size_t n, size_t offs = 0)
  {
    for (size_t i = 0; i < n; i++)
      v.push_back (i + offs);
    max = n-1 + offs;
  }
  T operator[] (size_t i) const { return v[i]; }
  T nonex() { return max + 1; }
  size_t find (T x)
  {
    auto it = std::find (v.begin(), v.end(), x);
    if (it != v.end()) return it - v.begin();
    return v.size();
  }
  T getnew()
  {
    v.push_back (++max);
    return max;
  }
    
  std::vector<T> v;
  T max;
};


template <class T>
struct Values<T*>
{
  Values (size_t n, size_t /*offs*/ = 0)
  {
    for (size_t i = 0; i < n; i++) {
      o.push_back (i);
      v.push_back (&o.back());
    }
    max = n-1;
  }
  T* operator[] (size_t i) const { return v[i]; }
  T* nonex() { return &dum; }
  size_t find (T* x)
  {
    auto it = std::find (v.begin(), v.end(), x);
    if (it != v.end()) return it - v.begin();
    return v.size();
  }
  T* getnew()
  {
    ++max;
    o.push_back (0);
    v.push_back (&o.back());
    return v.back();
  }
    
  std::vector<T*> v;
  std::deque<T> o;
  T dum = 0;
  size_t max = 0;
};


class Payload
{
public:
  constexpr static size_t MOVED = static_cast<size_t> (-1);
  Payload (size_t x = 0) : m_x (x)
  {
    ++s_count;
  }
  Payload (const Payload& other) : m_x (other.m_x)
  {
    ++s_count;
  }
  Payload (Payload&& other) : m_x (other.m_x)
  {
    other.m_x = MOVED;
    ++s_count;
  }
  Payload& operator= (const Payload&) = default;
  Payload& operator= (Payload&& other)
  {
    if (this != &other) {
      m_x = other.m_x;
      other.m_x = MOVED;
    }
    return *this;
  }
  ~Payload() { --s_count; }
  bool moved() const { return m_x == MOVED; }
  bool check (size_t x) const { return x == m_x; }
  size_t val() const { return m_x; }
  void setval (size_t x) { m_x = x; }
  bool operator== (const Payload& other) const { return m_x == other.m_x; }

  size_t m_x;
  static std::atomic<int> s_count;
};


std::atomic<int> Payload::s_count { 0 };


// Having an explicit symbol for this avoids parse errors with cppcheck 2.14.
static constexpr unsigned long minus1 = static_cast<unsigned long>(-1);
using TestMapu = CxxUtils::ConcurrentToValMap<unsigned long, Payload, TestUpdater,
                                              std::hash<unsigned long>,
                                              std::equal_to<unsigned long>,
                                              minus1>;
using TestMapi = CxxUtils::ConcurrentToValMap<int, Payload, TestUpdater,
                                              std::hash<int>,
                                              std::equal_to<int>,
                                              minus1>;
using TestMapp = CxxUtils::ConcurrentToValMap<int*, Payload, TestUpdater>;


template <class MAP>
void test1a()
{
  MAP map {typename MAP::Updater_t()};

  const size_t MAXKEYS = 1000;
  using key_type = typename MAP::key_type;
  Values<key_type> keys (MAXKEYS);

  assert (map.size() == 0);
  assert (map.capacity() == 64);
  assert (map.empty());

  for (size_t i = 0; i < MAXKEYS; i++) {
    auto [it, flag] = map.emplace (keys[i], Payload(i));
    assert (flag);
    assert (it.valid());
    assert (it->first == keys[i]);
    assert (it->second.check (i));
  }
  assert (Payload::s_count == MAXKEYS);

  assert (map.size() == MAXKEYS);
  assert (map.capacity() == 1024);
  assert (!map.empty());

  for (size_t i = 0; i < MAXKEYS; i++) {
    typename MAP::const_iterator it = map.find (keys[i]);
    assert (it.valid());
    assert (it != map.end());
    assert (it->first == keys[i]);
    assert (it->second.check (i));
  }
  assert (map.count (keys[10]) == 1);
  assert (map.count (keys.nonex()) == 0);
  assert (map.contains (keys[10]));
  assert (!map.contains (keys.nonex()));
  assert (map.find (keys.nonex()) == map.end());
  assert (!map.find (keys.nonex()).valid());

  assert (map.updater().inGrace() == 0x0e);

  {
    auto [i1, i2] = map.equal_range (keys[10]);
    assert (i1.valid());
    assert (i1 != i2);
    assert (i1->first == keys[10]);
    assert (i1->second.check (10));
    ++i1;
    assert (i1 == i2);
  }

  {
    auto [i1, i2] = map.equal_range (keys.nonex());
    assert (!i1.valid());
    assert (i1 == map.end());
    assert (i1 == i2);
  }

  assert (map.at (keys[10]).check (10));
  EXPECT_EXCEPTION (std::out_of_range, map.at (keys.nonex()));

  //----
  std::vector<size_t> exp;
  for (size_t i = 0; i < MAXKEYS; i++) {
    exp.push_back (i);
  }

  std::vector<size_t> seen;
  // not using reference, because our iterator doesn't return a reference
  for (const auto p : map.range()) {
    size_t i = p.second.val();
    assert (i < MAXKEYS);
    assert (p.first == keys[i]);
    seen.push_back (i);
  }

  std::sort (seen.begin(), seen.end());
  assert (seen == exp);

  seen.clear();
  // not using reference, because our iterator doesn't return a reference
  for (const auto p : map) {
    size_t i = p.second.val();
    assert (i < MAXKEYS);
    assert (p.first == keys[i]);
    seen.push_back (i);
  }

  std::sort (seen.begin(), seen.end());
  assert (seen == exp);

  key_type knew = keys.getnew();
  {
    auto [it, flag] = map.insert (std::make_pair (knew, Payload (keys.max)));
    assert (flag);
    assert (it.valid());
    assert (it->first == knew);
    assert (it->second.check (keys.max));
  }    
  assert (Payload::s_count == MAXKEYS+1);
  {
    auto [it, flag] = map.insert (std::make_pair (knew, Payload (keys.max+1)));
    assert (!flag);
    assert (it.valid());
    assert (it->first == knew);
    assert (it->second.check (keys.max));
  }
  assert (Payload::s_count == MAXKEYS+1);

  assert (map.size() == MAXKEYS+1);
  assert (map.capacity() == 1024);
  assert (!map.empty());

  {
    key_type knew2 = keys.getnew();
    Payload pnew2 (keys.max);
    assert (!pnew2.moved());
    auto [it, flag] = map.emplace (knew2, std::move (pnew2));
    assert (it.valid());
    assert (it->first == knew2);
    assert (it->second.check (keys.max));
    assert (pnew2.moved());
    assert (Payload::s_count == MAXKEYS+3);
  }
  assert (Payload::s_count == MAXKEYS+2);
  assert (map.size() == MAXKEYS+2);

  {
    key_type knew3 = keys.getnew();
    auto [it, flag] = map.emplace (knew3, std::make_unique<Payload> (keys.max));
    assert (it.valid());
    assert (it->first == knew3);
    assert (it->second.check (keys.max));
  }
  assert (Payload::s_count == MAXKEYS+3);
  assert (map.size() == MAXKEYS+3);

  {
    key_type knew4 = keys.getnew();
    auto p = std::make_pair (knew4, Payload (keys.max));
    assert (!p.second.moved());
    auto [it, flag] = map.insert (std::move (p));
    assert (it.valid());
    assert (it->first == knew4);
    assert (it->second.check (keys.max));
    assert (p.second.moved());
    assert (Payload::s_count == MAXKEYS+5);
  }
  assert (Payload::s_count == MAXKEYS+4);
  assert (map.size() == MAXKEYS+4);

  {
    key_type knew5 = keys.getnew();
    auto p = std::make_pair (knew5, std::make_unique<Payload> (keys.max));
    assert (p.second.get() != nullptr);
    auto [it, flag] = map.insert (std::move (p));
    assert (it.valid());
    assert (it->first == knew5);
    assert (it->second.check (keys.max));
    assert (p.second.get() == nullptr);
    assert (Payload::s_count == MAXKEYS+5);
  }
  assert (map.size() == MAXKEYS+5);
}


void test1()
{
  std::cout << "test1\n";
  test1a<TestMapu>();
  assert (Payload::s_count == 0);
  test1a<TestMapi>();
  assert (Payload::s_count == 0);
  test1a<TestMapp>();
  assert (Payload::s_count == 0);
}


// Bulk copy / insert.
template <class MAP>
void test2a()
{
  using Updater_t = typename MAP::Updater_t;
  using key_type = typename MAP::key_type;
  using mapped_type = typename MAP::mapped_type;
  using const_iterator = typename MAP::const_iterator;
  Values<key_type> keys (10);

  std::vector<std::pair<key_type, mapped_type> > data;
  for (size_t i = 0; i < 10; i++)
    data.emplace_back (keys[i], Payload(i));
  MAP map1 (data.begin(), data.end(), Updater_t());
  assert (!data[0].second.moved());
  assert (map1.size() == 10);
  assert (Payload::s_count == 20);
  {
    const_iterator it = map1.find (keys[4]);
    assert (it.valid());
    assert (it != map1.end());
    assert (it->first == keys[4]);
    assert (it->second.check (4));
  }

  MAP map1a (data.cbegin(), data.cend(), Updater_t());
  assert (!data[0].second.moved());
  assert (map1a.size() == 10);
  assert (map1a.at (keys[5]).check (5));
  assert (Payload::s_count == 30);

  MAP map2 (map1, Updater_t());
  assert (map2.size() == 10);
  assert (Payload::s_count == 40);
  {
    const_iterator it = map2.find (keys[6]);
    assert (it.valid());
    assert (it != map2.end());
    assert (it->first == keys[6]);
    assert (it->second.check (6));
  }

  MAP map3 {Updater_t()};
  assert (map3.capacity() == 64);
  assert (map3.size() == 0);

  map3.reserve (200);
  assert (map3.capacity() == 256);
  map3.reserve (100);
  assert (map3.capacity() == 256);
  map3.quiescent (Context_t());
  assert (map3.size() == 0);

  map3.insert (std::begin(data), std::end(data));
  assert (!data[0].second.moved());
  assert (map3.size() == 10);
  assert (map3.at (keys[5]).check (5));
  assert (Payload::s_count == 50);

  assert (map3.capacity() == 256);
  assert (!map3.empty());

  MAP map3a {Updater_t()};
  map3a.insert (std::cbegin(data), std::cend(data));
  assert (!data[0].second.moved());
  assert (map3a.size() == 10);
  assert (map3a.at (keys[5]).check (5));
  assert (Payload::s_count == 60);

  //----
  // Range insertions using move.

  Values<key_type> keys4 (10, 10);
  std::vector<std::pair<key_type, mapped_type> > data4;
  for (size_t i = 0; i < 10; i++)
    data4.emplace_back (keys4[i], Payload(i+10));
  for (size_t i = 0; i < 10; i++)
    assert (!data4[i].second.moved());
  assert (Payload::s_count == 70);

  MAP map4 {Updater_t()};
  map4.insert (std::make_move_iterator (std::begin(data4)),
               std::make_move_iterator (std::end(data4)));
  assert (map4.size() == 10);
  assert (Payload::s_count == 80);
  assert (map4.at (keys4[5]).check (15));
  for (size_t i = 0; i < 10; i++)
    assert (data4[i].second.moved());

  Values<key_type> keys5 (10, 20);
  std::vector<std::pair<key_type, std::unique_ptr<mapped_type> > > data5;
  for (size_t i = 0; i < 10; i++)
    data5.emplace_back (keys5[i], std::make_unique<Payload>(i+20));
  assert (Payload::s_count == 90);

  map4.insert (std::make_move_iterator (std::begin(data5)),
               std::make_move_iterator (std::end(data5)));
  assert (map4.size() == 20);
  assert (Payload::s_count == 90);
  assert (map4.at (keys5[5]).check (25));

  //----
  // Range ctors using move.

  Values<key_type> keys6 (10, 30);
  std::vector<std::pair<key_type, mapped_type> > data6;
  for (size_t i = 0; i < 10; i++)
    data6.emplace_back (keys6[i], Payload(i+30));
  for (size_t i = 0; i < 10; i++)
    assert (!data6[i].second.moved());
  assert (Payload::s_count == 100);

  MAP map6 {std::make_move_iterator (data6.begin()),
            std::make_move_iterator (data6.end()),
            Updater_t()};
  assert (map6.size() == 10);
  assert (Payload::s_count == 110);
  assert (map6.at (keys6[5]).check (35));
  for (size_t i = 0; i < 10; i++)
    assert (data6[i].second.moved());

  Values<key_type> keys7 (10, 40);
  std::vector<std::pair<key_type, std::unique_ptr<mapped_type> > > data7;
  for (size_t i = 0; i < 10; i++)
    data7.emplace_back (keys7[i], std::make_unique<Payload>(i+40));
  assert (Payload::s_count == 120);

  MAP map7 {std::make_move_iterator (data7.begin()),
            std::make_move_iterator (data7.end()),
            Updater_t()};
  assert (map7.size() == 10);
  assert (Payload::s_count == 120);
  assert (map7.at (keys7[5]).check (45));
}
void test2()
{
  std::cout << "test2\n";
  test2a<TestMapu>();
  assert (Payload::s_count == 0);
  test2a<TestMapi>();
  assert (Payload::s_count == 0);
  test2a<TestMapp>();
  assert (Payload::s_count == 0);
}


// Swap
template <class MAP>
void test_swap1()
{
  MAP map1 {typename MAP::Updater_t()};
  MAP map2 {typename MAP::Updater_t()};

  const size_t MAXKEYS = 1000;
  using key_type = typename MAP::key_type;
  using const_iterator = typename MAP::const_iterator;
  Values<key_type> keys1 (MAXKEYS);
  Values<key_type> keys2 (MAXKEYS/2, MAXKEYS);

  for (size_t i = 0; i < MAXKEYS; i++) {
    auto [it, flag] = map1.emplace (keys1[i], Payload(i));
    assert (flag);
  }
  for (size_t i = 0; i < MAXKEYS/2; i++) {
    auto [it, flag] = map2.emplace (keys2[i], Payload(i+MAXKEYS));
    assert (flag);
  }
  assert (Payload::s_count == MAXKEYS + MAXKEYS/2);

  assert (map1.size() == MAXKEYS);
  assert (map1.capacity() == 1024);
  assert (map2.size() == MAXKEYS/2);
  assert (map2.capacity() == 512);

  map1.swap (map2);

  assert (map1.size() == MAXKEYS/2);
  assert (map1.capacity() == 512);
  assert (map2.size() == MAXKEYS);
  assert (map2.capacity() == 1024);
  assert (Payload::s_count == MAXKEYS + MAXKEYS/2);

  for (size_t i = 0; i < MAXKEYS; i++) {
    const_iterator it = map2.find (keys1[i]);
    assert (it.valid());
    assert (it->first == keys1[i]);
    assert (it->second.check (i));
  }

  for (size_t i = 0; i < MAXKEYS/2; i++) {
    const_iterator it = map1.find (keys2[i]);
    assert (it.valid());
    assert (it->first == keys2[i]);
    assert (it->second.check (i + MAXKEYS));
  }
}
void test_swap()
{
  std::cout << "test_swap\n";
  test_swap1<TestMapu>();
  assert (Payload::s_count == 0);
  test_swap1<TestMapi>();
  assert (Payload::s_count == 0);
  test_swap1<TestMapp>();
  assert (Payload::s_count == 0);
}


template <class MAP>
void test_nonconst1()
{
  using key_type = typename MAP::key_type;
  Values<key_type> keys (1);
  MAP map1 {typename MAP::Updater_t()};
  map1.emplace (keys[0], Payload (1));
  map1.at(keys[0]).setval (3);

  const MAP& cmap1 = map1;
  assert (cmap1.at (keys[0]).val() == 3);

  map1.find (keys[0])->second.setval (5);
  assert (cmap1.at (keys[0]).val() == 5);

  map1.begin()->second.setval (7);
  assert (cmap1.at (keys[0]).val() == 7);

  map1.range().begin()->second.setval (9);
  assert (cmap1.at (keys[0]).val() == 9);

  map1.equal_range (keys[0]).first->second.setval (11);
  assert (cmap1.at (keys[0]).val() == 11);
}
// Test methods returning nonconst references to the mapped object.
void test_nonconst()
{
  std::cout << "test_nonconst\n";
  test_nonconst1<TestMapu>();
  assert (Payload::s_count == 0);
  test_nonconst1<TestMapi>();
  assert (Payload::s_count == 0);
  test_nonconst1<TestMapp>();
  assert (Payload::s_count == 0);
}


//***************************************************************************
// Threaded test.
//


std::shared_timed_mutex start_mutex;


template <class MAP>
class test4_Base
{
public:
  using key_type = typename MAP::key_type;
  using mapped_type = typename MAP::mapped_type;
  
  static constexpr size_t nwrites = 10000;
  
  test4_Base();
  const key_type key (size_t i) const { return m_keys[i]; }
  const mapped_type& val (size_t i) const { return m_vals[i]; }
  void setContext();


  Values<key_type> m_keys;
  std::vector<Payload> m_vals;
  key_type m_last_key;
  mapped_type m_last_val;
};


template <class MAP>
test4_Base<MAP>::test4_Base()
  : m_keys (nwrites),
    m_last_val (nwrites)
{
  m_vals.reserve (nwrites);
  for (size_t i = 0; i < nwrites; i++) {
    m_vals.emplace_back (i);
  }
  m_last_key = m_keys.nonex();
}


template <class MAP>
void test4_Base<MAP>::setContext()
{
}



template <class MAP>
class test4_Writer
{
public:
  test4_Writer (int slot, MAP& map, test4_Base<MAP>& base);
  void operator()();

private:
  int m_slot;
  MAP& m_map;
  test4_Base<MAP>& m_base;
};


template <class MAP>
test4_Writer<MAP>::test4_Writer (int slot, MAP& map, test4_Base<MAP>& base)
  : m_slot (slot),
    m_map (map),
    m_base (base)
{
}


template <class MAP>
void test4_Writer<MAP>::operator()()
{
  m_base.setContext();
  std::shared_lock<std::shared_timed_mutex> lock (start_mutex);

  static constexpr size_t nwrites = test4_Base<MAP>::nwrites;

  for (size_t i=0; i < nwrites; i++) {
    assert (m_map.emplace (m_base.key(i), m_base.val(i)).second);
    m_map.quiescent (m_slot);
    if (((i+1)%128) == 0) {
      usleep (1000);
    }
  }

  assert (m_map.emplace (m_base.m_last_key, m_base.m_last_val).second);
}


template <class MAP>
class test4_Iterator
{
public:
  test4_Iterator (int slot, MAP& map, test4_Base<MAP>& base);
  void operator()();

private:
  int m_slot;
  MAP& m_map;
  test4_Base<MAP>& m_base;
};


template <class MAP>
test4_Iterator<MAP>::test4_Iterator (int slot, MAP& map, test4_Base<MAP>& base)
  : m_slot (slot),
    m_map (map),
    m_base (base)
{
}


template <class MAP>
void test4_Iterator<MAP>::operator()()
{
  m_base.setContext();
  std::shared_lock<std::shared_timed_mutex> lock (start_mutex);

  static constexpr size_t nwrites = test4_Base<MAP>::nwrites;

  while (true) {
    // not using reference, because our iterator doesn't return a reference
    for (const auto p : m_map) {
      if (p.first == m_base.m_last_key) continue;
      size_t i = m_base.m_keys.find (p.first);
      assert (m_base.key(i) == p.first);
      assert (p.second == m_base.val(i));
    }

    typename MAP::const_iterator_range range = m_map.range();
    typename MAP::const_iterator begin2 = range.begin();
    typename MAP::const_iterator end2 = range.end();
    while (begin2 != end2) {
      --end2;
      if (end2->first == m_base.m_last_key) continue;
      size_t i = m_base.m_keys.find (end2->first);
      assert (m_base.key(i) == end2->first);
      assert (end2->second == m_base.val(i));
    }

    m_map.quiescent (m_slot);
    if (m_map.size() > nwrites) break;
  }
}


template <class MAP>
class test4_Reader
{
public:
  test4_Reader (int slot, MAP& map, test4_Base<MAP>& base);
  void operator()();

private:
  int m_slot;
  MAP& m_map;
  test4_Base<MAP>& m_base;
};


template <class MAP>
test4_Reader<MAP>::test4_Reader (int slot, MAP& map, test4_Base<MAP>& base)
  : m_slot (slot),
    m_map (map),
    m_base (base)
{
}


template <class MAP>
void test4_Reader<MAP>::operator()()
{
  m_base.setContext();
  std::shared_lock<std::shared_timed_mutex> lock (start_mutex);

  static constexpr size_t nwrites = test4_Base<MAP>::nwrites;

  while (true) {
    for (size_t i = 0; i < nwrites; ++i) {
      typename MAP::const_iterator it = m_map.find (m_base.key(i));
      if (it == m_map.end()) break;
      assert(it->second == m_base.val(i));
    }

    m_map.quiescent (m_slot);
    if (m_map.size() > nwrites) break;
  }
}


template <class MAP>
void test4_iter()
{
  MAP map {typename MAP::Updater_t()};

  const int nthread = 4;
  std::thread threads[nthread];
  start_mutex.lock();

  test4_Base<MAP> base;
  threads[0] = std::thread (test4_Writer<MAP> (0, map, base));
  threads[1] = std::thread (test4_Iterator<MAP> (1, map, base));
  threads[2] = std::thread (test4_Reader<MAP> (2, map, base));
  threads[3] = std::thread (test4_Reader<MAP> (3, map, base));

  // Try to get the threads starting as much at the same time as possible.
  start_mutex.unlock();
  for (int i=0; i < nthread; i++)
    threads[i].join();
}


void test_threaded()
{
  std::cout << "test_threaded\n";

  for (int i=0; i < 5; i++) {
    test4_iter<TestMapu>();
    assert (Payload::s_count == 0);
    test4_iter<TestMapi>();
    assert (Payload::s_count == 0);
    test4_iter<TestMapp>();
    assert (Payload::s_count == 0);
  }
}


int main (int /*argc*/, char** /*argv*/)
{
  std::cout << "CxxUtils/ConcurrentToValMap_test\n";
  test1();
  test2();
  test_swap();
  test_nonconst();
  test_threaded();
  return 0;
}
