// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/copy_bounded.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2013
 * @brief Copy a range with bounds restriction.
 */


#ifndef CXXUTILS_COPY_BOUNDED_H
#define CXXUTILS_COPY_BOUNDED_H


#include <iterator>
#include <algorithm>


namespace CxxUtils {


/**
 * @brief Copy a range with bounds restriction; generic version.
 */
template <class InputIterator, class OutputIterator,
          class InputTag, class OutputTag>
inline
OutputIterator
copy_bounded1 (InputIterator begi, InputIterator endi,
               OutputIterator bego, OutputIterator endo,
               const InputTag&,
               const OutputTag&)
{
  while (begi != endi && bego != endo) {
    *bego = *begi;
    ++begi;
    ++bego;
  }
  return bego;
}


/**
 * @brief Copy a range with bounds restriction; random_access_iterator version.
 */
template <class InputIterator, class OutputIterator>
inline
OutputIterator
copy_bounded1 (InputIterator begi, InputIterator endi,
               OutputIterator bego, OutputIterator endo,
               const std::random_access_iterator_tag&,
               const std::random_access_iterator_tag&)
{
  size_t n = std::min (endi-begi, endo-bego);
  return std::copy (begi, begi+n, bego);
}


/**
 * @brief Copy a range with bounds restriction.
 * @param begi Start of input range.
 * @param endi End of input range.
 * @param bego Start of output range.
 * @param endo End of output range.
 *
 * Like std::copy(begi, endi, bego), except that it will not copy
 * more than std::distance(bego, endo) elements.
 *
 * Copies exactly n = std::min (std::distance(begi,endi),
 *                              std::distance(bego,endo)) elements.
 * Returns bego + n.
 */
template <std::input_iterator InputIterator,
          std::output_iterator<typename std::iterator_traits<InputIterator>::value_type> OutputIterator>
inline
OutputIterator
copy_bounded (InputIterator begi, InputIterator endi,
              OutputIterator bego, OutputIterator endo)
{
  return copy_bounded1
    (begi, endi, bego, endo,
     typename std::iterator_traits<InputIterator>::iterator_category(),
     typename std::iterator_traits<OutputIterator>::iterator_category());
}


/**
 * @brief Copy a range with bounds restriction.
 * @param input Input range
 * @param output Output range
 *
 * copy_bounded written in terms of iterator ranges.
 */
template <class InputRange, class OutputRange>
inline
auto
copy_bounded (const InputRange& input, OutputRange& output)
  -> decltype (std::begin(output))
{
  return copy_bounded
    (std::begin(input),  std::end(input),
     std::begin(output), std::end(output));
}


/**
 * @brief Copy a range with bounds restriction.
 * @param input Input range
 * @param output Output range
 *
 * copy_bounded written in terms of iterator ranges.
 *
 * We have this as a distinct overload in order to be able to pass
 * a CxxUtils::span temporary as the second argument.
 */
template <class InputRange, class OutputRange>
inline
auto
copy_bounded (const InputRange& input, OutputRange&& output)
  -> decltype (std::begin(output))
{
  return copy_bounded
    (std::begin(input),  std::end(input),
     std::begin(output), std::end(output));
}


} // namespace CxxUtils



#endif // not CXXUTILS_COPY_BOUNDED_H
