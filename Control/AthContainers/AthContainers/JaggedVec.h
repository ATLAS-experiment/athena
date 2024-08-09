// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/JaggedVec.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2024
 * @brief Auxiliary variable type allowing storage as a jagged vector.
 *        That is, the payloads for all the DataVector elements are
 *        stored contiguously in a single vector.
 *
 * (As yet incomplete; further documentation to be added.)
 */


#ifndef ATHCONTAINERS_JAGGEDVEC_H
#define ATHCONTAINERS_JAGGEDVEC_H


#include "AthContainers/JaggedVecImpl.h"
#include "AthContainers/tools/JaggedVecVectorFactory.h"
#include "AthContainers/JaggedVecConstAccessor.h"
#include "AthContainers/JaggedVecAccessor.h"


#endif // not ATHCONTAINERS_JAGGEDVEC_H
