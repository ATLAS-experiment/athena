/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//*****************************************************************************
//  Filename : ZdcRawData.cxx
//  Author   : Peter Steinberg
//  Created  : March 2009
//
//  DESCRIPTION:
//     
//
//  HISTORY:
//    20 March 2009: Created
//
//  BUGS:
//    
//    
//
//*****************************************************************************

#include "ZdcEvent/ZdcRawData.h"


#include <iostream>

namespace{
  void
  printVector(const auto & container, const std::string & label, std::ostream & text){
    text << label;
    for (auto v:container){
      text << " " <<v;
    }
  }
}

ZdcRawData::ZdcRawData( const Identifier& id )
  : m_id (id)
{
}

void ZdcRawData::print() const
{
    std::cout << (std::string) (*this) << std::endl;
}

ZdcRawData::operator std::string() const
{
    std::string result(whoami());
    return result;    
}


void ZdcRawData::print_to_stream ( const std::vector<double>& val,
                                    const std::string & label,
                                    std::ostream & text)
{
    printVector(val, label, text);
}

void ZdcRawData::print_to_stream ( const std::vector<int>& val,
                                    const std::string & label,
                                    std::ostream & text)
{
    printVector(val, label, text);
}

