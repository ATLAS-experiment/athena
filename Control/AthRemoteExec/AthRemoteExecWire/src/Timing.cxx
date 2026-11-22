/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <AthRemoteExecWire/Timing.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numeric>
#include <sstream>

namespace AthRemoteExec {

namespace {

using Clock = RequestTiming::Clock;

float micros(Clock::time_point from, Clock::time_point to) {
  const float value =
      std::chrono::duration<float, std::micro>(to - from).count();
  return value > 0.f ? value : 0.f;
}

float percentile(const std::vector<float>& sorted, float fraction) {
  if (sorted.empty()) {
    return 0.f;
  }
  const size_t index = static_cast<size_t>(std::min<float>(
      sorted.size() - 1, std::floor(fraction * (sorted.size() - 1))));
  return sorted[index];
}

struct Phase {
  std::string_view name;
  Clock::time_point RequestTiming::* from;
  Clock::time_point RequestTiming::* to;
};

/// The phases, in the order a request meets them.
static constexpr Phase s_phases[] = {
    {"unmarshal   (proto -> plain)", &RequestTiming::handlerEntry,
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
    {"marshal     (plain -> proto)", &RequestTiming::replied,
     &RequestTiming::handlerExit},
};

}  // namespace

bool RequestTiming::complete() const {
  return handlerEntry != invalid && converted != invalid && taken != invalid &&
         recorded != invalid && pushed != invalid && finished != invalid &&
         replied != invalid && handlerExit != invalid;
}

TimingAccumulator::TimingAccumulator(size_t capacity) : m_capacity(capacity) {
  m_samples.reserve(m_capacity);
}

void TimingAccumulator::add(const RequestTiming& timing) {
  if (!timing.complete()) {
    return;
  }
  const std::lock_guard<std::mutex> lock(m_mutex);
  if (m_samples.size() >= m_capacity) {
    ++m_dropped;
    return;
  }
  m_samples.push_back(timing);
}

std::string TimingAccumulator::report() const {
  std::vector<RequestTiming> samples;
  size_t dropped = 0;
  {
    const std::lock_guard<std::mutex> lock(m_mutex);
    samples = m_samples;
    dropped = m_dropped;
  }
  if (samples.empty()) {
    return {};
  }

  std::ostringstream out;
  out << std::format(
      "in-process cost over {} request(s){}, microseconds:\n", samples.size(),
      dropped > 0 ? std::format(" ({} not sampled)", dropped) : "");
  out << std::format("  {:<34} {:>9} {:>9} {:>9} {:>9}\n", "phase", "mean",
                     "p50", "p99", "max");

  const auto row = [&out](std::string_view name, std::vector<float>& values) {
    std::sort(values.begin(), values.end());
    const float mean =
        std::accumulate(values.begin(), values.end(), 0.f) / values.size();
    out << std::format("  {:<34} {:>9.1f} {:>9.1f} {:>9.1f} {:>9.1f}\n",
                       name.data(), mean, percentile(values, 0.5),
                       percentile(values, 0.99), values.back());
  };

  std::vector<float> values;
  values.reserve(samples.size());
  for (const Phase& phase : s_phases) {
    values.clear();
    for (const RequestTiming& sample : samples) {
      values.push_back(micros(sample.*phase.from, sample.*phase.to));
    }
    row(phase.name, values);
  }

  out << "  " << std::string(34 + 4 * 10, '-') << '\n';

  values.clear();
  for (const RequestTiming& sample : samples) {
    values.push_back(micros(sample.handlerEntry, sample.handlerExit));
  }
  row("TOTAL       (handler in -> out)", values);

  values.clear();
  for (const RequestTiming& sample : samples) {
    values.push_back(micros(sample.handlerEntry, sample.handlerExit) -
                     micros(sample.pushed, sample.finished));
  }
  row("  with 'execute' removed", values);

  out << "  This timing excludes RPC's framework own parse and serialise,\n"
         "  which happen outside the handler and cannot be timed from inside\n"
         "  it. It also excludes: the gate algorithm decoding the payload,\n"
         "  the pack algorithm encoding the reply, the scheduler, and the\n"
         "  fragment itself.";

  return out.str();
}

}  // namespace AthRemoteExec
