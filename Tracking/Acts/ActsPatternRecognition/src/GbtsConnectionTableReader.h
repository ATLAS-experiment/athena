/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_GBTSCONNECTIONTABLEREADER_H
#define ACTSTRK_GBTSCONNECTIONTABLEREADER_H

#include <cstdint>
#include <iosfwd>
#include <vector>

namespace ActsTrk::GbtsConnectionTable {

/// One row of the table: the two layers it connects, and the stage it is in.
struct Connection {
  std::uint32_t stage{};
  std::uint32_t src{};
  std::uint32_t dst{};
};

/// The table as it is on file.
struct ReadResult {
  /// The eta bin width the table was made for.
  float etaBinWidth{};
  /// The connections, in file order.
  std::vector<Connection> connections;
};

/// Read the GBTS layer connection table.
///
/// The whole table, as written: which of it applies to the detector and to
/// the pass at hand is the caller's to decide. The bin table each connection
/// carries is skipped, GBTS recomputes it from the layer geometry.
///
/// @param inputStream the table
/// @throws std::runtime_error if the table cannot be read
/// @return the table
ReadResult read(std::istream& inputStream);

}  // namespace ActsTrk::GbtsConnectionTable

#endif
