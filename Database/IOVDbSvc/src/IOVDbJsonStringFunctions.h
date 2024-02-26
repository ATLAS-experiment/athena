/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
// @file IOVDbJsonStringFunctions.h
// @brief Helper functions for create string in JSON format
// @author Evgeny Alexandrov
// @date 14 Febrary 2024
#ifndef IOVDbSvc_IOVDbJsonStringFunctions_h
#define IOVDbSvc_IOVDbJsonStringFunctions_h

#include <string>
#include <iostream>
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeListSpecification.h"
#include "CoralBase/Attribute.h"

namespace IOVDbNamespace{
    //class methods
    ///Produce a representation of a coral::Attribute as a json string
    std::string jsonAttribute(const coral::Attribute&);

    ///Produce a representation of a coral::AttributeList as a json string
    std::string jsonAttributeList(const coral::AttributeList&);

    ///json open tag, '{'
    inline static const std::string s_openJson = "{";

    ///json close tag, '}'
    inline static const std::string s_closeJson = "}";

    ///json standard delimiter ', '
    inline static const std::string s_delimiterJson = ", ";
}
#endif

