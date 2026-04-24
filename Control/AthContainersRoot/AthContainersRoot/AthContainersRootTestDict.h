// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainersRoot/AthContainersRootTestDict.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2016
 * @brief For AthContainersRoot unit tests.
 */


#ifndef ATHCONTAINERSROOT_ATHCONTAINERSROOTTESTDICT_H
#define ATHCONTAINERSROOT_ATHCONTAINERSROOTTESTDICT_H


#include "AthContainersRoot/test/Foo.h"
#include "AthContainers/tools/AuxTypeVectorFactory.h"
#include "AthLinks/ElementLink.h"
#include "AthLinks/DataLink.h"
#include <vector>


namespace AthContainersRootTest {


class Bar {};
class Baz {};
class L1 {};
class L2 {};
class L3 {};


} // namespace AthContainersRootTest


template class std::vector<AthContainersRootTest::Foo*>;
template class ElementLink<std::vector<AthContainersRootTest::Foo*> >;
template class std::vector<ElementLink<std::vector<AthContainersRootTest::Foo*> > >;
template class std::vector<std::vector<ElementLink<std::vector<AthContainersRootTest::Foo*> > > >;
template class DataLink<std::vector<AthContainersRootTest::Foo*> >;
template class std::vector<DataLink<std::vector<AthContainersRootTest::Foo*> > >;
template class std::vector<std::vector<DataLink<std::vector<AthContainersRootTest::Foo*> > > >;

template class std::vector<AthContainersRootTest::Foo>;
template class ElementLink<std::vector<AthContainersRootTest::Foo> >;
template class std::vector<ElementLink<std::vector<AthContainersRootTest::Foo> > >;
template class std::vector<std::vector<ElementLink<std::vector<AthContainersRootTest::Foo> > > >;

template class std::vector<AthContainersRootTest::L1>;
template class std::vector<AthContainersRootTest::L2>;
template class std::vector<AthContainersRootTest::L3>;

namespace SG {
template class AuxTypeVectorFactory<AthContainersRootTest::Foo>;
template class AuxTypeVectorFactory<AthContainersRootTest::L2>;
}


#endif // not ATHCONTAINERSROOT_ATHCONTAINERSROOTTESTDICT_H
