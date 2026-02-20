/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/test/ConcurrentMap_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2023
 * @brief Tests for ConcurrentMap.
 */


#ifndef NO_TBB
# define HAVE_TBB
#endif


#undef NDEBUG

#include "CxxUtils/ConcurrentMap.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/expect_exception.h"
#include "TestTools/random.h"
#ifndef NO_PERF
# ifdef HAVE_TBB
#  include "tbb/concurrent_unordered_map.h"
# endif
# include "boost/timer/timer.hpp"
# ifdef HAVE_CK
extern "C" {
#  include "ck_ht.h"
}
# endif
#endif // not NO_PERF
#include <unordered_map>
#include <thread>
#include <shared_mutex>
#include <vector>
#include <deque>
#include <memory>
#include <mutex>
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
  void change (size_t i)
  {
    v[i] += 10000;
    max = std::max (max, v[i]);
  }
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
  }
  T* operator[] (size_t i) const { return v[i]; }
  T* nonex() { return &dum; }
  void change (size_t i)
  {
    o.push_back (0);
    v[i] = &o.back();
  }
  size_t find (T* x)
  {
    auto it = std::find (v.begin(), v.end(), x);
    if (it != v.end()) return it - v.begin();
    return v.size();
  }
  T* getnew()
  {
    o.push_back (0);
    v.push_back (&o.back());
    return v.back();
  }
    
  std::vector<T*> v;
  std::deque<T> o;
  T dum = 0;
};


// Having explicit symbols for these avoids parse errors with cppcheck 2.14.
static constexpr unsigned long minus1 = static_cast<unsigned long>(-1);
static constexpr unsigned long minus2 = static_cast<unsigned long>(-2);
using TestMapul = CxxUtils::ConcurrentMap<unsigned long, int, TestUpdater,
                                          std::hash<unsigned long>,
                                          std::equal_to<unsigned long>,
                                          minus1, minus2>;
using TestMapip = CxxUtils::ConcurrentMap<int, int*, TestUpdater,
                                          std::hash<int>,
                                          std::equal_to<int>,
                                          minus1, minus2>;
using TestMappu = CxxUtils::ConcurrentMap<int*, unsigned, TestUpdater,
                                          std::hash<int*>,
                                          std::equal_to<int*>,
                                          0, minus1>;
using TestMapuf = CxxUtils::ConcurrentMap<unsigned long, float, TestUpdater,
                                          std::hash<unsigned long>,
                                          std::equal_to<unsigned long>,
                                          minus1, minus2>;


template <class MAP>
void test1a()
{
  MAP map {typename MAP::Updater_t()};

  const size_t MAXKEYS = 1000;
  using key_type = typename MAP::key_type;
  using mapped_type = typename MAP::mapped_type;
  Values<key_type> keys (MAXKEYS);
  Values<mapped_type> vals (MAXKEYS);

  assert (map.size() == 0);
  assert (map.capacity() == 64);
  assert (map.empty());

  for (size_t i = 0; i < MAXKEYS; i++) {
    auto [it, flag] = map.emplace (keys[i], vals[i]);
    assert (flag);
    assert (it.valid());
    assert (it->first == keys[i]);
    assert (it->second == vals[i]);
  }

  assert (map.size() == MAXKEYS);
  assert (map.capacity() == 1024);
  assert (!map.empty());

  for (size_t i = 0; i < MAXKEYS; i++) {
    typename MAP::const_iterator it = map.find (keys[i]);
    assert (it.valid());
    assert (it != map.end());
    assert (it->first == keys[i]);
    assert (it->second == vals[i]);
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
    assert (i1->second == vals[10]);
    ++i1;
    assert (i1 == i2);
  }

  {
    auto [i1, i2] = map.equal_range (keys.nonex());
    assert (!i1.valid());
    assert (i1 == map.end());
    assert (i1 == i2);
  }

  assert (map.at (keys[10]) == vals[10]);
  EXPECT_EXCEPTION (std::out_of_range, map.at (keys.nonex()));

  {
    auto lock = map.lock();
    for (size_t i = 0; i < MAXKEYS; i++) {
      vals.change(i);
      auto [it, flag] = map.insert_or_assign (lock, keys[i], vals[i]);
      assert (!flag);
      assert (it.valid());
      assert (it->first == keys[i]);
      assert (it->second == vals[i]);
    }
  }

  assert (map.size() == MAXKEYS);
  assert (map.capacity() == 1024);

  for (size_t i = 0; i < MAXKEYS; i++) {
    typename MAP::const_iterator it = map.find (keys[i]);
    assert (it.valid());
    assert (it != map.end());
    assert (it->first == keys[i]);
    assert (it->second == vals[i]);
  }

  //----
  std::vector<size_t> exp;
  for (size_t i = 0; i < MAXKEYS; i++) {
    exp.push_back (i);
  }

  std::vector<size_t> seen;
  // not using reference, because our iterator doesn't return a reference
  for (const auto p : map.range()) {
    size_t i = vals.find (p.second);
    assert (i < MAXKEYS);
    assert (p.first == keys[i]);
    seen.push_back (i);
  }

  std::sort (seen.begin(), seen.end());
  assert (seen == exp);

  seen.clear();
  // not using reference, because our iterator doesn't return a reference
  for (const auto p : map) {
    size_t i = vals.find (p.second);
    assert (i < MAXKEYS);
    assert (p.first == keys[i]);
    seen.push_back (i);
  }

  std::sort (seen.begin(), seen.end());
  assert (seen == exp);

  key_type knew = keys.getnew();
  mapped_type vnew = vals.getnew();
  mapped_type vnew2 = vals.getnew();
  {
    auto [it, flag] = map.insert (std::make_pair (knew, vnew));
    assert (flag);
    assert (it.valid());
    assert (it->first == knew);
    assert (it->second == vnew);
  }    
  {
    auto [it, flag] = map.insert (std::make_pair (knew, vnew2));
    assert (!flag);
    assert (it.valid());
    assert (it->first == knew);
    assert (it->second == vnew);
  }

  assert (map.size() > MAXKEYS);
  assert (map.capacity() == 1024);
  assert (!map.empty());
  map.clear();
  assert (map.size() == 0);
  assert (map.capacity() == 1024);
  assert (map.empty());
  map.clear(2048);
  assert (map.size() == 0);
  assert (map.capacity() == 2048);
  assert (map.empty());
}


void test1()
{
  std::cout << "test1\n";
  test1a<TestMapul>();
  test1a<TestMapip>();
  test1a<TestMappu>();
  test1a<TestMapuf>();
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
  Values<mapped_type> vals (10);

  std::vector<std::pair<key_type, mapped_type> > data;
  for (size_t i = 0; i < 10; i++)
    data.emplace_back (keys[i], vals[i]);
  MAP map1 (data.begin(), data.end(), Updater_t());
  assert (map1.size() == 10);
  {
    const_iterator it = map1.find (keys[4]);
    assert (it.valid());
    assert (it != map1.end());
    assert (it->first == keys[4]);
    assert (it->second == vals[4]);
  }

  MAP map2 (map1, Updater_t());
  assert (map2.size() == 10);
  {
    const_iterator it = map2.find (keys[6]);
    assert (it.valid());
    assert (it != map2.end());
    assert (it->first == keys[6]);
    assert (it->second == vals[6]);
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
  assert (map3.size() == 10);
  assert (map3.at (keys[5]) == vals[5]);

  assert (map3.capacity() == 256);
  assert (!map3.empty());
  map3.forceClear();
  assert (map3.capacity() == 256);
  assert (map3.size() == 0);
  assert (map3.empty());
}
void test2()
{
  std::cout << "test2\n";
  test2a<TestMapul>();
  test2a<TestMapip>();
  test2a<TestMappu>();
}


// Erase
template <class MAP>
void test3a()
{
  MAP map {typename MAP::Updater_t()};

  const size_t MAXKEYS = 1000;
  using key_type = typename MAP::key_type;
  using mapped_type = typename MAP::mapped_type;
  using const_iterator = typename MAP::const_iterator;
  Values<key_type> keys (MAXKEYS);
  Values<mapped_type> vals (MAXKEYS);
  Values<mapped_type> vals2 (MAXKEYS, MAXKEYS);

  {
    auto lock = map.lock();
    for (size_t i = 0; i < MAXKEYS; i++) {
      auto [it, flag] = map.emplace (lock, keys[i], vals[i]);
      assert (flag);
      assert (it.valid());
      assert (it->first == keys[i]);
      assert (it->second == vals[i]);
    }
  }
  assert (map.size() == 1000);
  assert (map.capacity() == 1024);
  assert (map.erased() == 0);

  for (size_t i = 0; i < MAXKEYS; i+=2) {
    assert (map.erase (keys[i]));
  }
  assert (map.size() == 500);
  assert (map.capacity() == 1024);
  assert (map.erased() == 500);

  for (size_t i = 0; i < MAXKEYS; i+=2) {
    const_iterator it2 = map.find (keys[i]);
    if (i%2 == 0) {
      assert (!it2.valid());
    }
    else {
      assert (it2.valid());
      assert (it2->first == keys[i]);
      assert (it2->second == vals[i]);
    }
  }

  for (size_t i = 2; i < MAXKEYS; i+=4) {
    auto [it, flag] = map.emplace (keys[i], vals2[i]);
    assert (flag);
    assert (it.valid());
  }
  assert (map.size() == 750);
  assert (map.capacity() == 2048);
  assert (map.erased() == 0);

  for (size_t i = 0; i < MAXKEYS; i+=2) {
    const_iterator it2 = map.find (keys[i]);
    if (i%4 == 0) {
      assert (!it2.valid());
    }
    else {
      assert (it2.valid());
      assert (it2->first == keys[i]);
      assert (it2->second == vals2[i]);
    }
  }

  map.forceClear();
  assert (map.size() == 0);
  assert (map.capacity() == 2048);
  assert (map.erased() == 0);
  for (size_t i = 0; i < MAXKEYS; i++) {
    const_iterator it2 = map.find (keys[i]);
    assert (!it2.valid());
  }
}
void test3()
{
  std::cout << "test3\n";
  test3a<TestMapul>();
  test3a<TestMapip>();
  test3a<TestMappu>();
}


// Swap
template <class MAP>
void test_swap1()
{
  MAP map1 {typename MAP::Updater_t()};
  MAP map2 {typename MAP::Updater_t()};

  const size_t MAXKEYS = 1000;
  using key_type = typename MAP::key_type;
  using mapped_type = typename MAP::mapped_type;
  using const_iterator = typename MAP::const_iterator;
  Values<key_type> keys1 (MAXKEYS);
  Values<mapped_type> vals1 (MAXKEYS);
  Values<key_type> keys2 (MAXKEYS, MAXKEYS);
  Values<mapped_type> vals2 (MAXKEYS, MAXKEYS);

  for (size_t i = 0; i < MAXKEYS; i++) {
    auto [it, flag] = map1.emplace (keys1[i], vals1[i]);
    assert (flag);
  }
  for (size_t i = 0; i < MAXKEYS; i++) {
    auto [it, flag] = map2.emplace (keys2[i], vals2[i]);
    assert (flag);
  }
  {
    auto lock = map2.lock();
    for (size_t i = 0; i < MAXKEYS; i+=2) {
      assert (map2.erase (lock, keys2[i]));
    }
  }

  assert (map1.size() == MAXKEYS);
  assert (map1.capacity() == 1024);
  assert (map1.erased() == 0);
  assert (map2.size() == MAXKEYS/2);
  assert (map2.capacity() == 1024);
  assert (map2.erased() == MAXKEYS/2);

  map1.swap (map2);

  assert (map1.size() == MAXKEYS/2);
  assert (map1.capacity() == 1024);
  assert (map1.erased() == MAXKEYS/2);
  assert (map2.size() == MAXKEYS);
  assert (map2.capacity() == 1024);
  assert (map2.erased() == 0);

  for (size_t i = 0; i < MAXKEYS; i++) {
    const_iterator it = map2.find (keys1[i]);
    assert (it.valid());
    assert (it->first == keys1[i]);
    assert (it->second == vals1[i]);
  }

  for (size_t i = 0; i < MAXKEYS; i++) {
    const_iterator it = map1.find (keys2[i]);
    if ((i%2) == 0) {
      assert (!it.valid());
    }
    else {
      assert (it.valid());
      assert (it->first == keys2[i]);
      assert (it->second == vals2[i]);
    }
  }
}
void test_swap()
{
  std::cout << "test_swap\n";
  test_swap1<TestMapul>();
  test_swap1<TestMapip>();
  test_swap1<TestMappu>();
  test_swap1<TestMapuf>();
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
  const mapped_type val (size_t i) const { return m_vals[i]; }
  const mapped_type val2 (size_t i) const { return m_vals2[i]; }
  const mapped_type val3 (size_t i) const { return m_vals3[i]; }
  void setContext();


  Values<key_type> m_keys;
  Values<mapped_type> m_vals;
  Values<mapped_type> m_vals2;
  Values<mapped_type> m_vals3;
  key_type m_last_key;
  mapped_type m_last_val;
};


template <class MAP>
test4_Base<MAP>::test4_Base()
  : m_keys (nwrites),
    m_vals (nwrites),
    m_vals2 (nwrites, 10*nwrites),
    m_vals3 (nwrites, 20*nwrites)
{
  m_last_key = m_keys.nonex();
  m_last_val = m_vals3.nonex();
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

  for (size_t i=0; i < nwrites; i++) {
    assert (!m_map.insert_or_assign (m_base.key(i), m_base.val2(i)).second);
    m_map.quiescent (m_slot);
    if (((i+1)%128) == 0) {
      usleep (1000);
    }
  }

  for (size_t i=0; i < nwrites; i++) {
    assert (!m_map.insert_or_assign (m_base.key(i), m_base.val3(i)).second);
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
      assert (p.second == m_base.val(i)|| p.second == m_base.val2(i) || p.second == m_base.val3(i));
    }

    typename MAP::const_iterator_range range = m_map.range();
    typename MAP::const_iterator begin2 = range.begin();
    typename MAP::const_iterator end2 = range.end();
    while (begin2 != end2) {
      --end2;
      if (end2->first == m_base.m_last_key) continue;
      size_t i = m_base.m_keys.find (end2->first);
      assert (m_base.key(i) == end2->first);
      assert (end2->second == m_base.val(i) || end2->second == m_base.val2(i) || end2->second == m_base.val3(i));
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
      assert(it->second == m_base.val(i) || it->second == m_base.val2(i) || it->second == m_base.val3(i));
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
    test4_iter<TestMapul>();
    test4_iter<TestMapip>();
    test4_iter<TestMappu>();
  }
}


#ifndef NO_PERF
//***************************************************************************
// Optional performance test.
//


class ConcurrentMapAdapter
  : public TestMapul
{
public:
  ConcurrentMapAdapter()
    : TestMapul (TestMapul::Updater_t(), 512)
  {
  }
  static std::string name() { return "ConcurrentMap"; }

  int get (unsigned long k) const
  {
    auto it = find (k);
    if (it.valid()) return it->second;
    return 0;
  }

  static unsigned int key (const const_iterator& i)
  { return i->first; }
  static int value (const const_iterator& i)
  { return i->second; }
};


class UnorderedMapAdapter
{
public:
  typedef std::unordered_map<unsigned long, int> map_t;
  typedef map_t::const_iterator const_iterator;
  
  static std::string name() { return "UnorderedMap"; }

  void emplace (unsigned long k, int p)
  {
    lock_t lock (m_mutex);
    m_map.emplace (k, p);
  }


  int get (unsigned long k) const
  {
    lock_t lock (m_mutex);
    auto it = m_map.find (k);
    if (it != m_map.end()) return it->second;
    return 0;
  }

  const_iterator begin() const { return m_map.begin(); }
  const_iterator end() const { return m_map.end(); }
  static unsigned long key (const const_iterator& i)
  { return i->first; }
  static int value (const const_iterator& i)
  { return i->second; }

  
private:
  map_t m_map;

  typedef std::mutex mutex_t;
  typedef std::lock_guard<mutex_t> lock_t;
  mutable mutex_t m_mutex;
};


#ifdef HAVE_TBB
class ConcurrentUnorderedMapAdapter
{
public:
  typedef tbb::concurrent_unordered_map<unsigned long, int> map_t;
  typedef map_t::const_iterator const_iterator;

  static std::string name() { return "concurrent_unordered_map"; }

  void emplace (unsigned long k, int p)
  {
    m_map.emplace (k, p);
  }


  int get (unsigned long k) const
  {
    auto it = m_map.find (k);
    if (it != m_map.end()) return it->second;
    return 0;
  }

  const_iterator begin() const { return m_map.begin(); }
  const_iterator end() const { return m_map.end(); }
  static unsigned long key (const const_iterator& i)
  { return i->first; }
  static int value (const const_iterator& i)
  { return i->second; }

private:
  map_t m_map;
};
#endif


#ifdef HAVE_CK
class CKHTAdapter
{
public:
  CKHTAdapter();
  static std::string name() { return "ck_ht"; }

  struct const_iterator
  {
    const_iterator (ck_ht_t* ht)
      : m_ht (ht)
    {
      if (m_ht) {
        ck_ht_iterator_init (&m_it);
        if (!ck_ht_next (m_ht, &m_it, &m_entry)) {
          m_entry = nullptr;
        }
      }
      else {
        m_entry = nullptr;
      }
    }

    const_iterator& operator++()
    {
      if (!ck_ht_next (m_ht, &m_it, &m_entry)) {
        m_entry = nullptr;
      }
      return *this;
    }

    bool operator!= (const const_iterator& other) const
    {
      return m_entry != other.m_entry;
    }

    ck_ht_t* m_ht;
    ck_ht_iterator_t m_it;
    ck_ht_entry_t* m_entry;
  };

  void emplace (unsigned long k, int p)
  {
    ck_ht_entry_t entry;
    ck_ht_hash_t h;
    ck_ht_hash_direct (&h, &m_ht, static_cast<uintptr_t>(k));
    ck_ht_entry_set_direct (&entry, h, k, p);
    ck_ht_put_spmc (&m_ht, h, &entry);
  }


  int get (unsigned long k) const
  {
    ck_ht_entry_t entry;
    ck_ht_hash_t h;
    ck_ht_hash_direct (&h, &m_ht, k);
    ck_ht_entry_key_set_direct (&entry, k);
    if (ck_ht_get_spmc (&m_ht, h, &entry)) {
      uintptr_t ret = reinterpret_cast<uintptr_t>(ck_ht_entry_value (&entry));
      return ret;
    }
    return 0;
  }

  static unsigned long key (const const_iterator& i)
  {
    return reinterpret_cast<unsigned long> (ck_ht_entry_key_direct (i.m_entry));
  }
  static int value (const const_iterator& i)
  {
    uintptr_t ret = reinterpret_cast<uintptr_t>(ck_ht_entry_value(i.m_entry));
    return ret;
  }

  const_iterator begin() const
  {
    return const_iterator (&m_ht);
  }

  const_iterator end() const
  {
    return const_iterator (nullptr);
  }


private:
  static void hash (ck_ht_hash_t* h,
                    const void* key,
                    size_t /*key_length*/,
                    uint64_t /*seed*/)
  {
    static const std::hash<const void*> hasher;
    h->value = hasher (*reinterpret_cast<const void* const *>(key));
  }

  static void ht_free (void *p, size_t /*b*/, bool /*r*/) { free(p); }

  ck_malloc m_alloc;
  mutable ck_ht_t m_ht ATLAS_THREAD_SAFE;
};


CKHTAdapter::CKHTAdapter()
{
  m_alloc.malloc = malloc;
  m_alloc.free = ht_free;
  if (!ck_ht_init (&m_ht,
                   CK_HT_MODE_DIRECT,
                   hash, // ck_ht_hash_cb_t
                   &m_alloc, // ck_malloc*
                   128, // initial size
                   6602834))
  {
    std::cout << "ck_hs_init error\n";
  }
}
#endif // HAVE_CK


class Timer
{
public:
  Timer();

  class RunTimer
  {
  public:
    RunTimer (boost::timer::cpu_timer& timer) : m_timer (&timer)
    { timer.resume(); }
    RunTimer (RunTimer&& other) : m_timer (other.m_timer) { other.m_timer = nullptr; }
    ~RunTimer() { if (m_timer) m_timer->stop(); }
  private:
    boost::timer::cpu_timer* m_timer;
  };
  RunTimer run() { return RunTimer (m_timer); }

  std::string format() const { return m_timer.format(3); }

private:
  boost::timer::cpu_timer m_timer;
};


Timer::Timer()
{
  m_timer.stop();
}


class TesterBase
{
public:
  TesterBase();

  Timer::RunTimer run_lookup_timer() { return m_lookup_timer.run(); }
  Timer::RunTimer run_iterate_timer() { return m_iterate_timer.run(); }

  void report();

private:
  Timer m_lookup_timer;
  Timer m_iterate_timer;
};


TesterBase::TesterBase()
{
}


void TesterBase::report()
{
  std::cout << "lookup:  " << m_lookup_timer.format();
  std::cout << "iterate: " << m_iterate_timer.format();
}


template <class CONT>
class Tester
  : public TesterBase
{
public:
  static constexpr size_t NCONT = 1000;
  static constexpr size_t NEACH = 10000;
  static constexpr size_t LOG_NENT = 11;
  static constexpr size_t NENT = 1<<LOG_NENT;
  static constexpr size_t ENT_MASK = NENT-1;
  
  Tester();
  void lookup_test();
  void iterate_test();

  void test();
  std::string name() { return CONT::name(); }

private:
  std::vector<unsigned int> m_keys[NCONT];
  CONT m_cont[NCONT];
  uint32_t m_seed;
};


template <class CONT>
Tester<CONT>::Tester()
  : m_seed (1235)
{
  for (size_t j=0; j < NCONT; j++) {
    for (size_t i = 0; i < NENT; i++) {
      m_keys[j].push_back (j*NENT + i + 1);
    }
    for (size_t i = 0; i < NENT; i++) {
      m_cont[j].emplace (m_keys[j][i], m_keys[j][i] + 54321);
    }
  }
}


template <class CONT>
void Tester<CONT>::lookup_test()
{
  auto timer = run_lookup_timer();
  for (size_t irep = 0; irep < NEACH; irep++) {
    for (size_t icont = 0; icont < NCONT; icont++) {
      uint32_t ient = Athena_test::rng_seed (m_seed) & ENT_MASK;
      unsigned int key = m_keys[icont][ient];
      int val = m_cont[icont].get (key);
      assert (val == static_cast<int> (key + 54321));
    }
  }
}


template <class CONT>
void Tester<CONT>::iterate_test()
{
  auto timer = run_iterate_timer();
  int icount = 0;
  for (size_t irep = 0; irep < 100; irep++) {
    for (size_t icont = 0; icont < NCONT; icont++) {
      const CONT& cont = m_cont[icont];
      typename CONT::const_iterator it = cont.begin();
      typename CONT::const_iterator end = cont.end();
      while (it != end) {
        unsigned int key = CONT::key (it);
        int  val= CONT::value (it);
        if (((++icount) % 128) == 0) {
          assert (val == static_cast<int> (key + 54321));
        }
        ++it;
      }
    }
  }
}


template <class CONT>
void Tester<CONT>::test()
{
  lookup_test();
  iterate_test();
}


template <class CONT>
void perftest_one()
{
  // Tester<> is large... allocate it from the heap rather than
  // from the stack.
  auto tester = std::make_unique<Tester<CONT >>();
  std::cout << tester->name() << "\n";
  tester->test();
  tester->report();
}


void perftest()
{
  perftest_one<ConcurrentMapAdapter>();
  perftest_one<UnorderedMapAdapter>();
#ifdef HAVE_TBB
  perftest_one<ConcurrentUnorderedMapAdapter>();
#endif
#ifdef HAVE_CK
  perftest_one<CKHTAdapter>();
#endif
}
#endif // not NO_PERF


int main (int argc, char** argv)
{
  if (argc >= 2 && strcmp (argv[1], "--perf") == 0) {
#ifdef NO_PERF
    std::cout << " Performance tests disabled\n";
#else
    perftest();
#endif
    return 0;
  }

  std::cout << "CxxUtils/ConcurrentMap_test\n";
  test1();
  test2();
  test3();
  test_swap();
  test_threaded();
  return 0;
}
