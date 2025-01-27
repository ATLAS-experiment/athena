/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/PixelClusterdEdxCondData.h"

PixelClusterdEdxCondData::PixelClusterdEdxCondData() : m_var() {
  }

PixelClusterdEdxCondData::~PixelClusterdEdxCondData() = default;

std::string PixelClusterdEdxCondData::getVar() const {
  return m_var;
  }

void PixelClusterdEdxCondData::setVar(const std::string& value){
  m_var = value;
  return;
  }
