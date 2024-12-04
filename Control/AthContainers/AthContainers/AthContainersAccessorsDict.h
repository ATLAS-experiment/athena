// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/AthContainersAccessorDict.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2024
 * @brief Dictionary instantiations: Accessor classes.
 */


#ifndef ATHCONTAINERS_ATHCONTAINERSACCESSORDICT_H
#define ATHCONTAINERS_ATHCONTAINERSACCESSORDICT_H


#include "AthContainers/Accessor.h"
#include "AthContainers/ConstAccessor.h"
#include "AthContainers/Decorator.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/tools/AtomicConstAccessor.h"
#include <string>
#include <vector>


#define ARGS1 (const std::string&)
#define ARGS2 (const std::string&, const std::string&)
#define INSTAN_TYPE(TYP) \
  template class SG::ConstAccessor<TYP>; \
  template class SG::Accessor<TYP>; \
  template class SG::Decorator<TYP>; \
  template TYP& SG::AuxElement::auxdata<TYP> ARGS1; \
  template TYP& SG::AuxElement::auxdata<TYP> ARGS2; \
  template const TYP& SG::ConstAuxElement::auxdata<TYP> ARGS1 const; \
  template const TYP& SG::ConstAuxElement::auxdata<TYP> ARGS2 const; \
  template const TYP& SG::ConstAuxElement::auxdataConst<TYP> ARGS1 const; \
  template const TYP& SG::ConstAuxElement::auxdataConst<TYP> ARGS2 const; \
  template TYP& SG::ConstAuxElement::auxdecor<TYP> ARGS1 const; \
  template TYP& SG::ConstAuxElement::auxdecor<TYP> ARGS2 const;

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

template class SG::AtomicConstAccessor<unsigned int>;


#undef ARGS1
#undef ARGS2
#undef INSTAN_TYPE


#endif // not ATHCONTAINERS_ATHCONTAINERSACCESSORDICT_H
