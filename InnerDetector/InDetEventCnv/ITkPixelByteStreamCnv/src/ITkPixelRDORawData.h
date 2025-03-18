/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 03/2025
* Description: Initial implementation of the ITk Pixel RDO,
* heavily adapted from the existing ID
*/


#ifndef ITKPIXELRDORAWDATA_H
#define ITKPIXELRDORAWDATA_H

#include "InDetRawData/InDetRawData.h"


class ITkPixelRDORawData :   public InDetRawData{

    public:

        // Constructor with parameters:
        // offline compact id of the readout channel, 
        // the word
        ITkPixelRDORawData(const Identifier rdoId, const unsigned int word) : InDetRawData(rdoId,word){};
        ITkPixelRDORawData();

        virtual int getToT() const = 0;    // Time over Threshold value 0-255
        virtual int getBCID() const = 0;   // Beam Crossing ID
        virtual int getLVL1A() const = 0; // Level 1 accept, 0-15, used if reading 
        virtual int getLVL1ID() const = 0;  // ATLAS LVL1 0-255
  

};

#endif

