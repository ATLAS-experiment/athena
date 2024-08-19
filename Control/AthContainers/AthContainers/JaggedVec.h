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
 * When we want to store an auxiliary variable with a variable number
 * of data items per element, one usually uses a @c std::vector<T>.
 * This means that the auxiliary variable itself is stored as a nested
 * @c std::vector<std::vector<T> >.  This tends to be somewhat
 * inefficient in space, as well as being not so friendly if one wants
 * to share the data with heterogenous resources such as GPUs.
 *
 * A jagged vector flattens this structure.  The variable itself
 * contains simply a pair of indices into another, `linked', variable,
 * which holds the items for all container elements in a single
 * contiguous block.  Specialized accessor classes allow one to treat
 * the data as if it were still a separate vector per element.
 *
 * The usual way of declaring a jagged vector variable in an xAOD
 * container class is to use the macro defined in xAODCore/JaggedVec.h:
 *@code
 *  #include "xAODCore/PackedLink.h"
 *  ...
 *  class FooAuxContainer_v1 : public xAOD::AuxContainerBase {
 *  ...
 *  private:
 *    AUXVAR_JAGGEDVEC_DECL (int, intVec);
 *  };
 @endcode
 * This will declare an auxiliary variable @c intVec of type
 * @c SG::JaggedVecElt<int> containing the indices of the vector
 * elements, as well as a variable @c intVec_linked of type @c int
 * containing the actual vector data.
 *
 * One does not usually interact directly with the linked variable
 * or even with the @c SG::JaggedVecElt type.  Rather, the usual
 * Accessor classes provide a vector-like interface.  So one can write,
 * for example,
 *@code
 *  Foo& foo = ...;
 *  static const SG::Accessor<SG::JaggedVecElt<int> > intVecAcc ("intVec");
 *  size_t foo_sz = intVecAcc (foo).size();
 *  int foo_elt = intVecAcc (foo).at(1);
 *  for (int& ii : intVecElt (foo)) ... ;
 *  intVecElt (foo).push_back (10);
 *  intVecElt (foo).append_range (std::vector<int> {...});
 *  std::vector<int> v = intVecElt (foo);
 @endcode
 *
 * @c ConstAccessor and @c Decorator should work as expected.  The accessors
 * can also be used to create dynamic jagged vector variables.  A jagged
 * vector should be able to hold any type that can be put in a @c std::vector.
 * In particular, @c std::string and @c ElementLink have been tested.
 * However, if you are not using a primitive type, you may need to add
 * dictionary entries in order to save the variable.
 *
 * As mentioned above, you don't usually need to interact with the @c _linked
 * variable holding the actual payload.  However, you may need to name it
 * if you are selecting individual variables to write.
 */


#ifndef ATHCONTAINERS_JAGGEDVEC_H
#define ATHCONTAINERS_JAGGEDVEC_H


#include "AthContainers/JaggedVecImpl.h"
#include "AthContainers/tools/JaggedVecVectorFactory.h"
#include "AthContainers/JaggedVecConstAccessor.h"
#include "AthContainers/JaggedVecAccessor.h"
#include "AthContainers/JaggedVecDecorator.h"


#endif // not ATHCONTAINERS_JAGGEDVEC_H
