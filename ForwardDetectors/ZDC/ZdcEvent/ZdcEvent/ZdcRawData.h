/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
// Filename : ZdcRawData.h
// Author   : Peter Steinberg
// Created  : March 2009
//
// DESCRIPTION:
//    A ZdcRawData is the base class for raw data classes,
//    such as ZdcRdo
//    It has only one member element - HWIdentifier
//
// HISTORY:
//    20 March 2009
//
// BUGS:
//
// ***************************************************************************

#ifndef ZDCEVENT_ZDCRAWDATA_H
#define ZDCEVENT_ZDCRAWDATA_H

#include "Identifier/HWIdentifier.h"

#include <string>
#include <vector>
#include <iosfwd>

class ZdcRawData
{
public:

    /* Constructor: */
    ZdcRawData() = default;
    ZdcRawData(const Identifier& id);
    //copy constructor
    ZdcRawData(const ZdcRawData & z) noexcept = default;
    //move constructor
    ZdcRawData(ZdcRawData && z) noexcept = default;
    /* Destructor */
    virtual ~ZdcRawData() = default;
    ///Copy assignment
    ZdcRawData& operator=(ZdcRawData & z) noexcept = default;
    ///Move assignment
    ZdcRawData& operator=(ZdcRawData && z) noexcept = default;
    
    /*  Inline accessor methods: */
    inline Identifier   identify()  const   { return m_id;   }


    virtual std::string whoami   () const   { return "ZdcRawData"; }
    virtual void        print    () const;
    // Conversion operator to a std::string 
    // Can be used in a cast operation : (std::string) ZdcRawData
    virtual operator std::string() const;
  
    static void print_to_stream ( const std::vector<double>& val,
                                  const std::string & label, 
                                  std::ostream & text);

    static void print_to_stream ( const std::vector<int>& val,
                                  const std::string & label, 
                                  std::ostream & text);
private:

    Identifier m_id{}; // Hardware (online) ID of the adc
};

#endif  //ZDCEVENT_ZDCRAWDATA_H

