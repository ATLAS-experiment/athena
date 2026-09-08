/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AtlasHepMC/src/AttributeNames.cxx
 */


// Set up definitions for the variables declared in AttributeNames.h.
#define ATLASHEPMC_ATTRIBNAME(N) extern const std::string N{#N}
#include "AtlasHepMC/AttributeNames.h"
