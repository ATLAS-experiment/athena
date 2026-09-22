/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RemoteExecTiming.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numeric>
#include <sstream>

namespace AthExRemoteExec {

namespace {

using Clock = RequestTiming::Clock;

/// Microseconds between two stamps, floored at zero. steady_clock is monotonic
/// so a negative gap means a phase was never stamped, which complete() rejects;
/// the clamp is belt and braces for the report.
double micros( Clock::time_point from, Clock::time_point to )
{
  const double value =
      std::chrono::duration<double, std::micro>( to - from ).count();
  return value > 0.0 ? value : 0.0;
}

double percentile( std::vector<double>& sorted, double fraction )
{
  if ( sorted.empty() ) {
    return 0.0;
  }
  const size_t index = static_cast<size_t>(
      std::min<double>( sorted.size() - 1,
                        std::floor( fraction * ( sorted.size() - 1 ) ) ) );
  return sorted[index];
}

struct Phase {
  const char* name;
  Clock::time_point RequestTiming::*from;
  Clock::time_point RequestTiming::*to;
};

/// The phases, in the order a request meets them. Names say what the time was
/// spent on rather than which function it was in, because the point is to be
/// readable by somebody who has not read this package.
const Phase s_phases[] = {
    {"unmarshal   (protobuf -> plain)", &RequestTiming::handlerEntry,
     &RequestTiming::converted},
    {"queue       (waiting for loop)", &RequestTiming::converted,
     &RequestTiming::taken},
    {"dispatch    (slot + store + ctx)", &RequestTiming::taken,
     &RequestTiming::recorded},
    {"push        (to scheduler)", &RequestTiming::recorded,
     &RequestTiming::pushed},
    {"execute     (scheduler + algs)", &RequestTiming::pushed,
     &RequestTiming::finished},
    {"reply       (read back + encode)", &RequestTiming::finished,
     &RequestTiming::replied},
    {"marshal     (plain -> protobuf)", &RequestTiming::replied,
     &RequestTiming::handlerExit},
};

}  // anonymous namespace

bool RequestTiming::complete() const
{
  // Every stamp is set from steady_clock::now(), so a default-constructed
  // (epoch) value is the marker for "this phase never happened".
  const Clock::time_point unset{};
  return handlerEntry != unset && converted != unset && taken != unset &&
         recorded != unset && pushed != unset && finished != unset &&
         replied != unset && handlerExit != unset;
}

TimingAccumulator::TimingAccumulator( size_t capacity ) : m_capacity( capacity )
{
}

void TimingAccumulator::add( const RequestTiming& timing )
{
  if ( !timing.complete() ) {
    return;
  }
  const std::lock_guard<std::mutex> lock( m_mutex );
  if ( m_samples.size() >= m_capacity ) {
    ++m_dropped;
    return;
  }
  m_samples.push_back( timing );
}

size_t TimingAccumulator::count() const
{
  const std::lock_guard<std::mutex> lock( m_mutex );
  return m_samples.size();
}

size_t TimingAccumulator::dropped() const
{
  const std::lock_guard<std::mutex> lock( m_mutex );
  return m_dropped;
}

std::string TimingAccumulator::report() const
{
  std::vector<RequestTiming> samples;
  size_t dropped = 0;
  {
    const std::lock_guard<std::mutex> lock( m_mutex );
    samples = m_samples;
    dropped = m_dropped;
  }
  if ( samples.empty() ) {
    return {};
  }

  std::ostringstream out;
  char line[256];

  std::snprintf( line, sizeof( line ),
                 "in-process cost over %zu request(s)%s, microseconds:",
                 samples.size(),
                 dropped > 0
                     ? ( " (" + std::to_string( dropped ) + " not sampled)" )
                           .c_str()
                     : "" );
  out << line << '\n';
  std::snprintf( line, sizeof( line ), "  %-34s %9s %9s %9s %9s", "phase",
                 "mean", "p50", "p99", "max" );
  out << line << '\n';

  const auto row = [&]( const char* name, std::vector<double>& values ) {
    std::sort( values.begin(), values.end() );
    const double mean =
        std::accumulate( values.begin(), values.end(), 0.0 ) / values.size();
    std::snprintf( line, sizeof( line ), "  %-34s %9.1f %9.1f %9.1f %9.1f",
                   name, mean, percentile( values, 0.5 ),
                   percentile( values, 0.99 ), values.back() );
    out << line << '\n';
  };

  std::vector<double> values;
  values.reserve( samples.size() );
  for ( const Phase& phase : s_phases ) {
    values.clear();
    for ( const RequestTiming& sample : samples ) {
      values.push_back( micros( sample.*phase.from, sample.*phase.to ) );
    }
    row( phase.name, values );
  }

  out << "  " << std::string( 34 + 4 * 10, '-' ) << '\n';

  // The total is what the exercise is for: everything this process does with a
  // request between gRPC handing it over and gRPC taking it back.
  values.clear();
  for ( const RequestTiming& sample : samples ) {
    values.push_back( micros( sample.handlerEntry, sample.handlerExit ) );
  }
  row( "TOTAL       (handler in -> out)", values );

  // And the same with the scheduler's own time removed, which is the number
  // that stays meaningful when the payload algorithm is not a stand-in.
  values.clear();
  for ( const RequestTiming& sample : samples ) {
    values.push_back( micros( sample.handlerEntry, sample.handlerExit ) -
                      micros( sample.pushed, sample.finished ) );
  }
  row( "  of which not 'execute'", values );

  out << "  Excludes gRPC's own parse and serialise, which happen outside the\n"
         "  handler and cannot be timed from inside it. Note also that\n"
         "  'execute' is not pure orchestration: it contains the gate\n"
         "  algorithm decoding the payload and the pack algorithm encoding the\n"
         "  reply, as well as the scheduler and the fragment itself. Compare\n"
         "  runs at different payload sizes to separate them.";
  return out.str();
}

}  // namespace AthExRemoteExec
