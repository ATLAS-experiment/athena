/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 03/2025
* Description: Initial implementation of the ITk Pixel RDO,
* heavily adapted from the existing ID
*/



#ifndef ITKPIXEL1RAWDATA_H
#define ITKPIXEL1RAWDATA_H

// Base class
#include "ITkPixelRDORawData.h"


class ITkPixel1RawData final : public ITkPixelRDORawData{

public:
  // Constructor with parameters:
  // offline hashId of the readout element, 
  // the word

 ITkPixel1RawData();
 ITkPixel1RawData(const Identifier rdoId, const unsigned int word) : ITkPixelRDORawData(rdoId, word){};
 //
 ITkPixel1RawData(const ITkPixel1RawData&) = default;
 ITkPixel1RawData(ITkPixel1RawData&&) noexcept = default;
 ITkPixel1RawData& operator=(const ITkPixel1RawData&) = default;
 ITkPixel1RawData& operator=(ITkPixel1RawData&&) noexcept = default;
 virtual ~ITkPixel1RawData() = default;

 // Constructor with full parameter list: hashId, ToT, BCO ID,
 // LVL1 accept, ATLAS wide LVL1
 ITkPixel1RawData(const Identifier rdoId, const unsigned int ToT,
               const unsigned int BCID, const unsigned int LVL1ID,
               const unsigned int LVL1A = 0) : ITkPixelRDORawData(rdoId,
                                                                  ((ToT&0xFF)<<0)
                                                                  +((BCID&0xFF)<<8)
                                                                  +((LVL1ID&0xFF)<<16)
                                                                  +((LVL1A&0xF)<<24)){};

 ///////////////////////////////////////////////////////////////////
 // Virtual methods
 ///////////////////////////////////////////////////////////////////

 virtual int getToT() const override;    // Time over Threshold value 0-255
 virtual int getBCID() const override;   // Beam Crossing ID
 virtual int getLVL1A() const override;  // Level 1 accept, 0-15, used if
                                         // reading consecutive BCOs
 virtual int getLVL1ID() const override;  // ATLAS LVL1 0-255
};

///////////////////////////////////////////////////////////////////
// Inline methods:
///////////////////////////////////////////////////////////////////
// decode TOT information (taken from Calvet RawData class) 
inline int ITkPixel1RawData::getToT() const
{
  return (m_word & 0xFF);
}

// decode BCID information 
inline int ITkPixel1RawData::getBCID() const
{
  return ( (m_word>>8) & 0xFF);
}

// decode LVL1 accept information 
inline int ITkPixel1RawData::getLVL1A() const
{
  return ( (m_word>>24) & 0xF);
}

// decode Atlas wide LVL1 information 
inline int ITkPixel1RawData::getLVL1ID() const
{
  return ( (m_word>>16) & 0xFF);
}

#endif
