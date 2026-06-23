/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "PixelConditionsData/ITkPixFieldMaps.h"

ITkPixFieldMaps::ITkPixFieldMaps():
  m_lorentzMap_e(),
  m_lorentzMap_h(),
  m_distanceMap_e(),
  m_distanceMap_h(),
  m_ramoPotentialMap()
{ }

ITkPixFieldMaps::~ITkPixFieldMaps() = default;

// Map for radiation damage simulation

void ITkPixFieldMaps::setLorentzMap_e(std::vector<PixelHistoConverter> lorentzMap_e) { m_lorentzMap_e = std::move(lorentzMap_e); }
void ITkPixFieldMaps::setLorentzMap_h(std::vector<PixelHistoConverter> lorentzMap_h) { m_lorentzMap_h = std::move(lorentzMap_h); }
void ITkPixFieldMaps::setDistanceMap_e(std::vector<PixelHistoConverter> distanceMap_e) { m_distanceMap_e = std::move(distanceMap_e); }
void ITkPixFieldMaps::setDistanceMap_h(std::vector<PixelHistoConverter> distanceMap_h) { m_distanceMap_h = std::move(distanceMap_h); }
void ITkPixFieldMaps::setRamoPotentialMap(std::vector<PixelHistoConverter> ramoPotentialMap) { m_ramoPotentialMap = std::move(ramoPotentialMap); }

const PixelHistoConverter& ITkPixFieldMaps::getLorentzMap_e(int layer) const { return m_lorentzMap_e.at(layer); }
const PixelHistoConverter& ITkPixFieldMaps::getLorentzMap_h(int layer) const { return m_lorentzMap_h.at(layer); }
const PixelHistoConverter& ITkPixFieldMaps::getDistanceMap_e(int layer) const { return m_distanceMap_e.at(layer); }
const PixelHistoConverter& ITkPixFieldMaps::getDistanceMap_h(int layer) const { return m_distanceMap_h.at(layer); }
const PixelHistoConverter& ITkPixFieldMaps::getRamoPotentialMap(int layer) const { return m_ramoPotentialMap.at(layer); }


void ITkPixFieldMaps::clear() {
  m_lorentzMap_e.clear();
  m_lorentzMap_h.clear();
  m_distanceMap_e.clear();
  m_distanceMap_h.clear();
  m_ramoPotentialMap.clear();
}

