/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "Identifier/Identifiable.h"
#include "Identifier/HWIdentifier.h"
#include "Identifier/IdentifierHash.h"
#include "Identifier/IdHelper.h"

// default implementation
IdentifierHash	Identifiable::identifyHash() const{
    IdentifierHash result;
    return (result);
}

// default implementation
const IdHelper* 
Identifiable::getHelper() const{
    static_assert(std::is_trivially_destructible<IdentifierHash>::value);
    static_assert(std::is_trivially_copy_constructible<IdentifierHash>::value);
    static_assert(std::is_trivially_destructible<HWIdentifier>::value);
    static_assert(std::is_trivially_copy_constructible<HWIdentifier>::value);
    return (nullptr);
}

