// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/AthContainersJaggedVecsDict.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2024
 * @brief Dictionary instantiations: JaggedVec classes.
 */


#ifndef ATHCONTAINERS_ATHCONTAINERSJAGGEDVECSDICT_H
#define ATHCONTAINERS_ATHCONTAINERSJAGGEDVECSDICT_H


#include "AthContainers/JaggedVec.h"
#include <vector>
#include <string>


#define INSTAN_TYPE(TYP) \
  template class SG::JaggedVecElt<TYP>; \
  template class std::vector<SG::JaggedVecElt<TYP> >

INSTAN_TYPE(char);
INSTAN_TYPE(unsigned char);
INSTAN_TYPE(int);
INSTAN_TYPE(short);
INSTAN_TYPE(long);
INSTAN_TYPE(unsigned int);
INSTAN_TYPE(unsigned short);
INSTAN_TYPE(unsigned long);
INSTAN_TYPE(unsigned long long);
INSTAN_TYPE(float);
INSTAN_TYPE(double);
INSTAN_TYPE(bool);
INSTAN_TYPE(std::string);

INSTAN_TYPE(std::vector<char>);
INSTAN_TYPE(std::vector<unsigned char>);
INSTAN_TYPE(std::vector<int>);
INSTAN_TYPE(std::vector<short>);
INSTAN_TYPE(std::vector<long>);
INSTAN_TYPE(std::vector<unsigned int>);
INSTAN_TYPE(std::vector<unsigned short>);
INSTAN_TYPE(std::vector<unsigned long>);
INSTAN_TYPE(std::vector<unsigned long long>);
INSTAN_TYPE(std::vector<float>);
INSTAN_TYPE(std::vector<double>);
INSTAN_TYPE(std::vector<bool>);
INSTAN_TYPE(std::vector<std::string>);


#endif // not ATHCONTAINERS_ATHCONTAINERSJAGGEDVECSDICT_H
