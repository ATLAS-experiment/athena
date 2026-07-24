// Copyright (c) 2024 CERN for the benefit of the FastCaloSim project

#pragma once

#include <RtypesCore.h>
#include <TString.h>

#include <array>
#include <cfloat>
#include <cmath>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "FCAL_ChannelMap.h"
#include "FastCaloSim/Geometry/CaloGeo.h"

namespace FCALGeo {

class FCal : public CaloGeo {
 public:
  FCal() = default;

  virtual ~FCal() = default;

  /// Set the backing calorimeter cell store.
  void set_geo(CaloGeo* geo) { m_geo = geo; }

  /// Return the cell at a position in an FCal layer.
  auto get_cell(unsigned int layer, const Position& pos) const -> const Cell& {
    unsigned long long cell_id =
        this->get_fcal_cell_id(layer, pos.x(), pos.y(), pos.z());

    if (cell_id == static_cast<unsigned long long>(-1)) {
      static const Cell invalid_cell;
      return invalid_cell;
    }
    if (!m_geo) {
      throw std::logic_error("FCal cell store is not configured");
    }
    return m_geo->get_cell(cell_id);
  }

  /// Load the three FCal module maps.
  auto load(const std::array<std::string, 3>& fileNames) -> void {
    std::vector<std::unique_ptr<std::istream>> electrodes;
    electrodes.reserve(3);

    for (uint16_t i = 0; i < 3; i++) {
      const std::string& file = fileNames.at(i);

      std::unique_ptr<std::ifstream> directStream =
          std::make_unique<std::ifstream>(file);
      if (!directStream->is_open()) {
        throw std::runtime_error("Could not open FCal geometry file: " + file);
      }
      electrodes.push_back(std::move(directStream));
    }

    int thisTubeId;
    int thisTubeI;
    int thisTubeJ;
    double thisTubeX;
    double thisTubeY;
    TString tubeName;

    std::string seventh_column;
    std::string eight_column;
    int ninth_column;

    for (int imodule = 1; imodule <= 3; imodule++) {
      auto& input = *electrodes[imodule - 1];
      std::size_t tubeCount = 0;
      while (input >> tubeName >> thisTubeId >> thisTubeI >> thisTubeJ
                   >> thisTubeX >> thisTubeY >> seventh_column
                   >> eight_column >> ninth_column) {
        tubeName.ReplaceAll("'", "");
        m_FCal_ChannelMap.add_tube(tubeName.Data(), imodule, thisTubeId,
                                   thisTubeI, thisTubeJ, thisTubeX, thisTubeY,
                                   seventh_column);
        ++tubeCount;
      }
      if (!input.eof()) {
        throw std::runtime_error("Malformed FCal geometry file: "
                                 + fileNames[imodule - 1]);
      }
      if (tubeCount == 0) {
        throw std::runtime_error("Empty FCal geometry file: "
                                 + fileNames[imodule - 1]);
      }
    }

    m_FCal_ChannelMap.finish();

    this->calculateFCalRminRmax();
  }

  /// Calculate the radial extent of each FCal module.
  void calculateFCalRminRmax() {
    m_FCal_rmin.resize(3, FLT_MAX);
    m_FCal_rmax.resize(3, 0.);

    double x(0.), y(0.), r(0.);
    for (int imap = 1; imap <= 3; imap++)
      for (auto it = m_FCal_ChannelMap.begin(imap);
           it != m_FCal_ChannelMap.end(imap); it++) {
        x = it->second.x();
        y = it->second.y();
        r = std::sqrt(x * x + y * y);
        if (r < m_FCal_rmin[imap - 1])
          m_FCal_rmin[imap - 1] = r;
        if (r > m_FCal_rmax[imap - 1])
          m_FCal_rmax[imap - 1] = r;
      }
  }

  auto getClosestFCalCellIndex(int layer, float x, float y, int& ieta,
                               int& iphi, int* steps) const -> bool {
    if (layer < 21 || layer > 23) {
      return false;
    }
    double rmin = m_FCal_rmin[layer - 21];
    double rmax = m_FCal_rmax[layer - 21];
    int isam = layer - 20;
    double a = 1.;
    const double b = 0.01;
    const int nmax = 100;
    int i = 0;

    const double r = std::sqrt(x * x + y * y);
    if (r == 0.)
      return false;
    const double r_inverse = 1. / r;

    if ((r / rmax) > (rmin * r_inverse)) {
      x = x * rmax * r_inverse;
      y = y * rmax * r_inverse;
      while ((!m_FCal_ChannelMap.getTileID(isam, a * x, a * y, ieta, iphi)) &&
             i < nmax) {
        a -= b;
        i++;
      }
    } else {
      x = x * rmin * r_inverse;
      y = y * rmin * r_inverse;
      while ((!m_FCal_ChannelMap.getTileID(isam, a * x, a * y, ieta, iphi)) &&
             i < nmax) {
        a += b;
        i++;
      }
    }
    if (steps)
      *steps = i + 1;
    return i < nmax;
  }

  /// Return the best matching FCal cell for a hit.
  auto get_fcal_cell_id(int layer, double x, double y, double z,
                        int* steps = nullptr) const -> unsigned long long {
    if (layer < 21 || layer > 23) {
      return static_cast<unsigned long long>(-1);
    }
    int isam = layer - 20;
    int iphi(-100000), ieta(-100000);
    Long64_t mask1[]{0x34, 0x34, 0x35};
    Long64_t mask2[]{0x36, 0x36, 0x37};
    bool found = m_FCal_ChannelMap.getTileID(isam, x, y, ieta, iphi);
    if (steps && found)
      *steps = 0;
    if (!found) {
      // Project hits outside the active area onto the nearest cell.
      found = getClosestFCalCellIndex(layer, x, y, ieta, iphi, steps);
    }
    if (!found) {
      return static_cast<unsigned long long>(-1);
    }
    Long64_t id = (ieta << 5) + 2 * iphi;
    if (isam == 2)
      id += (8 << 8);

    if (z > 0)
      id += (mask2[isam - 1] << 12);
    else
      id += (mask1[isam - 1] << 12);

    id = id << 44;

    return id;
  }

 protected:
  // Keep the coordinates consistent with the ATLAS FCal channel map:
  // https://atlas-sw-doxygen.web.cern.ch/atlas-sw-doxygen/atlas_main--Doxygen/docs/html/d6/d72/FCALChannelMapBuilder_8cxx_source.html
  FCAL_ChannelMap m_FCal_ChannelMap{0};
  std::vector<double> m_FCal_rmin, m_FCal_rmax;

  // Non-owning backing cell store.
  CaloGeo* m_geo{nullptr};
};

}  // namespace FCALGeo
