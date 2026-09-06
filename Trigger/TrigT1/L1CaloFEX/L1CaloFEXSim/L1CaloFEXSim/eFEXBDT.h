/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXBDT .h
//                          -------------------
//     begin                : 14 07 2022
//     email                : david.reikher@cern.ch
//  **************************************************************************

#include "L1CaloFEXSim/conifer.h"
#include "nlohmann/json.hpp"
#include <fstream>

struct BDTVariable {
  std::string m_name;
  std::vector<std::vector<int>> m_scells;

public:
  friend void from_json(const nlohmann::json &j, BDTVariable &o) {
    j.at("name").get_to(o.m_name);
    j.at("scells").get_to(o.m_scells);
  }
};

class eFEXBDT {

private:
  std::vector<BDTVariable> m_variables;
  std::vector<std::vector<int>> m_et_scells;
  std::vector<std::vector<int>> m_em_et_scells;
  std::vector<std::vector<int>> m_had_et_scells;
  std::vector<std::vector<std::vector<int>>> m_towers_scells;
  conifer::BDT<unsigned int, unsigned int> m_bdt;
  int m_score_precision;

public:
  // Define how to read this class to/from JSON
  friend void from_json(const nlohmann::json &j, eFEXBDT &o) {
    j.at("variables").get_to(o.m_variables);
    j.at("et_scells").get_to(o.m_et_scells);
    j.at("em_et_scells").get_to(o.m_em_et_scells);
    j.at("had_et_scells").get_to(o.m_had_et_scells);
    j.at("towers_scells").get_to(o.m_towers_scells);
    j.at("score_precision").get_to(o.m_score_precision);
    j.at("bdt").get_to(o.m_bdt);
  }

  eFEXBDT(const std::string & filename) {
    /* Construct the BDT from conifer cpp backend JSON file */
    std::ifstream ifs(filename);
    nlohmann::json j = nlohmann::json::parse(ifs);
    from_json(j, *this);
    m_bdt.init();
  }

  const std::vector<BDTVariable> &getVariables() const { return m_variables; }

  const std::vector<std::vector<int>> &getETSCells() const {
    return m_et_scells;
  }

  const std::vector<std::vector<int>> &getEMETSCells() const {
    return m_em_et_scells;
  }

  const std::vector<std::vector<int>> &getHADETSCells() const {
    return m_had_et_scells;
  }

  const conifer::BDT<unsigned int, unsigned int> &getBDT() const {
    return m_bdt;
  }

  const std::vector<std::vector<int>> &getTowerSCells(int towerIndex) const {
    return m_towers_scells[towerIndex];
  }

  int getNTowers() const { return m_towers_scells.size(); }

  int getScorePrecision() const { return m_score_precision; }
};
