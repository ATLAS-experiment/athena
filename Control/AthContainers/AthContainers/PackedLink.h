// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/PackedLink.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2023
 * @brief Auxiliary variable type allowing to store @c ElementLinks
 *        as packed 32-bit values.
 *
 * Packed links allow representing @c ElementLinks as an auxiliary variable
 * that is a 32-bit integer.  Individual links are stored as a 24-bit
 * index and an 8-bit container index, which is an index into a linked
 * variable of DataLinks.  This saves space, but will add time overhead
 * when accessing the variable due to conversions back and forth
 * from @c ElementLink.
 *
 * The usual way of declaring a packed link variable in an xAOD
 * container class is to use the macros defined in xAODCore/PackedLink.h:
 *@code
 *  #include "xAODCore/PackedLink.h"
 *  ...
 *  class FooAuxContainer_v1 : public xAOD::AuxContainerBase {
 *  ...
 *  private:
 *    AUXVAR_PACKEDLINK_DECL (Container, clink);
 *    AUXVAR_PACKEDLINKVEC_DECL (std::vector, Container, cvec);
 *  };
 @endcode
 * This will declare the auxilary variable @c clink to be a packed
 * link to elements of @c Container, and @c cvec to ba a vector
 * of such packed links.
 *
 * Packed link variables are accessed using @c Accesor classes as usual,
 * where the type is given as @c SG::PackedLink<Container>.  So one
 * can write, for example,
 *@code
 *  Foo& foo = ...;
 *  static const SG::Accessor<SG::PackedLink<Container> > clinkAcc ("clink");
 *  ElementLink<Container> l = clinkAcc (foo);
 *  size_t index = clinkAcc (foo).index();
 *  clinkAcc(foo) = ElementLink<Container> (...);
 *
 *  static const SG::Accessor<std::vector<SG::PackedLink<Container> > >
 *                                                             cvecAcc ("cvec");
 *  size_t sz = cvecAcc (foo).size();
 *  ElementLink<Container> l2 = cvecAcc (foo)[1];
 *  for (ElementLink<Container> l3 = cvecAcc (foo)) { ... }
 *  std::vector<ElementLink<Container> > lvec = cvecAcc (foo);
 *  cvecAcc (foo)[1] = ElementLink<Container> (...);
 *  cvecAcc (foo) = std::vector<ElementLink<Container> > (...);
 @endcode
 *
 * @c ConstAccessor and @c Decorator should work as expected.  The accessors
 * can also be used to create dynamic packed link variables.
 *
 * For each packed link variable, there is an additional `linked' variable
 * created automatically.  In the example above, these would be named
 * @c clink_linked and @c cvec_linked.  Normally you don't need to refer
 * to these directly, but you may need to name them if you are selecting
 * individual variables to write.
 */


#ifndef ATHCONTAINERS_PACKEDLINK_H
#define ATHCONTAINERS_PACKEDLINK_H


#include "AthContainers/PackedLinkImpl.h"
#include "AthContainers/tools/PackedLinkVectorFactory.h"
#include "AthContainers/PackedLinkConstAccessor.h"
#include "AthContainers/PackedLinkAccessor.h"
#include "AthContainers/PackedLinkDecorator.h"


#endif // not ATHCONTAINERS_PACKEDLINK_H
