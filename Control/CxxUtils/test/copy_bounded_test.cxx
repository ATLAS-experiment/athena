/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/test/copy_bounded_test.cxx
 * @author sss
 * @date March 2013
 * @brief Regression tests for copy_bounded
 */


#undef NDEBUG

#include "CxxUtils/copy_bounded.h"
#include "CxxUtils/span.h"
#include <vector>
#include <list>
#include <cassert>
#include <iostream>
#include <ranges>
#include <algorithm>


struct arange
{
  static const int N = 10;
  int x[N];
  typedef int* iterator;
  typedef const int* const_iterator;
  typedef int value_type;
  iterator begin() { return x; }
  iterator end() { return x + N; }
  const_iterator begin() const { return x; }
  const_iterator end() const { return x + N; }

  arange(int) :x() {}
};



template <class InputRange, class OutputRange>
void test1b (InputRange& input, OutputRange& output)
{
  using InputIterator = std::ranges::iterator_t<InputRange>;
  using OutputIterator = std::ranges::iterator_t<OutputRange>;
  InputIterator begi = std::begin(input);
  InputIterator endi = std::end(input);
  OutputIterator bego = std::begin(output);
  OutputIterator endo = std::end(output);

  typedef typename std::iterator_traits<InputIterator>::value_type value_type;

  InputIterator midi = begi;
  std::advance (midi, 5);

  OutputIterator mido = bego;
  std::advance (mido, 5);

  std::ranges::subrange<InputIterator> rangei (begi, midi);
  std::ranges::subrange<OutputIterator> rangeo (bego, mido);

  int i = 0;
  for (value_type& it : input)
    it = i++;

  std::ranges::fill (output, 0);
  CxxUtils::copy_bounded (begi, endi, bego, mido);
  i = 0;
  for (value_type& it : output)
    assert (it == (i < 5 ? i++ : 0));

  std::ranges::fill (output, 0);
  CxxUtils::copy_bounded (input, rangeo);
  i = 0;
  for (value_type& it : output)
    assert (it == (i < 5 ? i++ : 0));

  std::ranges::fill (output, 0);
  CxxUtils::copy_bounded (begi, midi, bego, endo);
  i = 0;
  for (value_type& it : output)
    assert (it == (i < 5 ? i++ : 0));

  std::ranges::fill (output, 0);
  CxxUtils::copy_bounded (rangei, output);
  i = 0;
  for (value_type& it : output)
    assert (it == (i < 5 ? i++ : 0));

  std::ranges::fill (output, 0);
  CxxUtils::copy_bounded (begi, endi, bego, endo);
  i = 0;
  for (value_type& it : output)
    assert (it == i++);

  std::ranges::fill (output, 0);
  CxxUtils::copy_bounded (input, output);
  i = 0;
  for (value_type& it : output)
    assert (it == i++);
}

template <class Cont1, class Cont2=Cont1>
struct test1a
{
  static void test()
  {
    Cont1 c1 (10);
    Cont2 c2 (10);
    test1b (c1, c2);
  }
};


void test1()
{
  std::cout << "test1\n";
  test1a<std::vector<int> >::test();
  test1a<std::list<int> >::test();
  test1a<std::vector<int>, std::list<int> >::test();
  test1a<std::list<int>,   std::vector<int> >::test();

  test1a<arange>::test();
  test1a<arange, std::list<int> >::test();
  test1a<arange, std::vector<int> >::test();
  test1a<std::list<int>, arange>::test();
  test1a<std::vector<int>, arange>::test();

  std::vector<int> v1 {1, 2, 3, 4};
  std::vector<int> v2 (4);
  const std::vector<int>& cv1 = v1;
  CxxUtils::copy_bounded (CxxUtils::make_span (cv1), v2);
  assert (v1 == v2);
  v2[2] = 10;
  CxxUtils::copy_bounded (v2, CxxUtils::make_span (v1));
  assert (v1 == v2);
}

int main()
{
  test1();
  return 0;
}


