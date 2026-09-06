/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "src/GbtsConnectionTableReader.h"

#include <istream>
#include <stdexcept>
#include <string>

namespace ActsTrk {

GbtsConnectionTable::ReadResult GbtsConnectionTable::read(
    std::istream& inputStream) {
  ReadResult result;

  std::uint32_t nConnections{};
  std::uint32_t iConnection{};

  const auto fail = [&nConnections, &iConnection](const std::string& what) {
    throw std::runtime_error("GBTS connection table: cannot read " + what +
                             " at connection " + std::to_string(iConnection) +
                             " of " + std::to_string(nConnections));
  };

  if (!(inputStream >> nConnections >> result.etaBinWidth)) {
    throw std::runtime_error(
        "GBTS connection table: cannot read the connection count and the eta "
        "bin width");
  }

  for (; iConnection < nConnections; ++iConnection) {
    std::uint32_t index{};
    std::uint32_t height{};
    std::uint32_t width{};
    std::uint32_t nEntries{};
    Connection connection;

    if (!(inputStream >> index >> connection.stage >> connection.src >>
          connection.dst >> height >> width >> nEntries)) {
      fail("the header");
    }

    // GBTS recomputes the bin table from the layer geometry
    const std::uint64_t nBins = std::uint64_t{height} * width;
    for (std::uint64_t bin = 0; bin < nBins; ++bin) {
      std::uint32_t unused{};
      if (!(inputStream >> unused)) {
        fail("the bin table");
      }
    }

    result.connections.push_back(connection);
  }

  return result;
}

}  // namespace ActsTrk
