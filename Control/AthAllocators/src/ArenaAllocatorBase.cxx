/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file  AthAllocators/src/ArenaAllocatorBase.cxx
 * @author scott snyder
 * @date May 2007
 * @brief Common base class for arena allocator classes.
 *        Out-of-line implementations.
 */

#include "AthAllocators/ArenaAllocatorBase.h"
#include <ostream>
#include <iomanip>


namespace SG {


/**
 * @brief Constructor for a single statistic.
 */
ArenaAllocatorBase::Stats::Stat::Stat()
  : inuse (0),
    free (0),
    total (0)
{
}


/**
 * @brief Zero a statistic.
 */
void ArenaAllocatorBase::Stats::Stat::clear()
{
  inuse = free = total = 0;
}


/**
 * @brief Accumulate a statistic.
 * @param other The statistic to accumulate into this one.
 */
ArenaAllocatorBase::Stats::Stat&
ArenaAllocatorBase::Stats::Stat::operator+= (const Stat& other)
{
  inuse += other.inuse;
  free += other.free;
  total += other.total;
  return *this;
}


/* (hide from doxygen)
 * @brief Format a statistic structure.
 * @param os The stream to which to write.
 * @param stat The statistic structure to write.
 */
std::ostream& operator<< (std::ostream& os,
                          const ArenaAllocatorBase::Stats::Stat& stat)
{
  std::print (os, "{}", stat);
  return os;
}


//===========================================================================


/**
 * @brief Zero a complete statistics block.
 */
void ArenaAllocatorBase::Stats::clear()
{
  elts.clear();
  bytes.clear();
  blocks.clear();
}


/**
 * @brief Accumulate a complete statistics block.
 * @param other The statistics block to accumulate into this one.
 */
ArenaAllocatorBase::Stats&
ArenaAllocatorBase::Stats::operator+= (const Stats& other)
{
  elts += other.elts;
  bytes += other.bytes;
  blocks += other.blocks;
  return *this;
}


/* (hide from doxygen)
 * @brief Format a complete statistics structure.
 * @param os The stream to which to write.
 * @param stats The statistics structure to write.
 */
std::ostream& operator<< (std::ostream& os,
                          const ArenaAllocatorBase::Stats& stats)
{
  std::print (os, "{}", stats);
  return os;
}


/**
 * @brief Write a header for the statistics report.
 * @param os The stream to which to write.
 */
void ArenaAllocatorBase::Stats::header (std::ostream& os)
{
  std::print (os, "Elts InUse/Free/Total"
              "   Bytes InUse/Free/Total  Blocks InUse/Free/Total");
}


//===========================================================================


/**
 * @brief Generate a report on the memory usage of this allocator.
 * @param os Stream to which the report should be written.
 */
void ArenaAllocatorBase::report (std::ostream& os) const
{
  std::println (os, " {}  {}", stats(), name());
}


} // namespace SG


/// C++20 style formatters.
template <class FmtContext>
typename FmtContext::iterator
std::formatter<SG::ArenaAllocatorBase::Stats::Stat>::format (const SG::ArenaAllocatorBase::Stats::Stat& stat, FmtContext& ctx) const
{
  return std::format_to (ctx.out(), "{:7}/{:7}/{:7}", stat.inuse, stat.free, stat.total);
}


/// C++20 style formatters.
template <class FmtContext>
typename FmtContext::iterator
std::formatter<SG::ArenaAllocatorBase::Stats>::format (const SG::ArenaAllocatorBase::Stats& stats, FmtContext& ctx) const
{
  return std::format_to (ctx.out(), "{} {} {}", stats.elts, stats.bytes, stats.blocks);
}
