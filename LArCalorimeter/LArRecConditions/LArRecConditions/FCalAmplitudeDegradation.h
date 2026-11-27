/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARRECCONDITIONS_FCALAMPLITUDEDEGRADATION_H
#define LARRECCONDITIONS_FCALAMPLITUDEDEGRADATION_H

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

/**
 * Amplitude degradation factor D(channel, mu) of FCal1 from ion build-up at high luminosity
 * (L. Kurchaninov, Note-FCalSig, 2022). Channel = FCal1 eta index, mu = average mu of the lumi block.
 * Used in digitization (signal * D) and in reconstruction (energy / D).
 */
class FCalAmplitudeDegradation {
public:
  /// muGrid: increasing, first point above 0; table: one row per channel, one column per mu point.
  /// Returns an empty string if OK, otherwise the error message.
  std::string set(const std::vector<double>& muGrid, const std::vector<double>& table)
  {
    m_grid.clear();
    m_table.clear();
    m_nChannels = 0;
    const size_t nMu = muGrid.size();
    if (nMu == 0 || table.empty() || table.size() % nMu != 0) {
      return "table size " + std::to_string(table.size()) +
             " is not a non-zero multiple of the mu grid size " + std::to_string(nMu);
    }
    if (muGrid.front() <= 0.) return "mu grid must start above 0";
    for (size_t i = 1; i < nMu; ++i) {
      if (muGrid[i] <= muGrid[i - 1]) return "mu grid must be increasing";
    }
    for (double d : table) {
      if (!(d > 0. && d <= 1.)) {
        return "degradation factor " + std::to_string(d) + " is outside (0, 1]";
      }
    }
    m_grid = muGrid;
    m_table = table;
    m_nChannels = table.size() / nMu;
    return "";
  }

  size_t nChannels() const { return m_nChannels; }
  size_t nMuPoints() const { return m_grid.size(); }

  /// Linear interpolation in mu. Below the first point: linear from D = 1 at mu = 0.
  /// Above the last point: last value, no extrapolation.
  /// No table, channel out of range or mu not finite: 1 (no degradation).
  double factor(size_t channel, double mu) const
  {
    const size_t nMu = m_grid.size();
    if (nMu == 0 || channel >= m_nChannels || !std::isfinite(mu)) return 1.;
    const double* row = m_table.data() + channel * nMu;
    if (mu <= m_grid.front()) {
      return 1. + (row[0] - 1.) * std::max(mu, 0.) / m_grid.front();
    }
    if (mu >= m_grid.back()) {
      return row[nMu - 1];
    }
    size_t i = 1;
    while (m_grid[i] < mu) ++i;
    const double f = (mu - m_grid[i - 1]) / (m_grid[i] - m_grid[i - 1]);
    return row[i - 1] + f * (row[i] - row[i - 1]);
  }

private:
  std::vector<double> m_grid;
  std::vector<double> m_table;
  size_t m_nChannels{0};
};

#endif // LARRECCONDITIONS_FCALAMPLITUDEDEGRADATION_H
