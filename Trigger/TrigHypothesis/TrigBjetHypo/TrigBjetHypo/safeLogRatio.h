/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#pragma once

#include <cmath>
#include "xAODJet/Jet.h"
#include "xAODBTagging/BTagging.h"

float safeLogRatio(float num, float denom);

const xAOD::Jet* getJetFromBTagLink( const SG::AuxElement& btag );