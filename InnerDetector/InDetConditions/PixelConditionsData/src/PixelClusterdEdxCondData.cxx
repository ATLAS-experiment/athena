/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/PixelClusterdEdxCondData.h"

PixelClusterdEdxCondData::PixelClusterdEdxCondData() : m_var() {
  }

PixelClusterdEdxCondData::~PixelClusterdEdxCondData() = default;

int PixelClusterdEdxCondData::getVar() const {
  return m_var;
  }

void PixelClusterdEdxCondData::setVar(const int& value){
  m_var = value;
  return;
  }
