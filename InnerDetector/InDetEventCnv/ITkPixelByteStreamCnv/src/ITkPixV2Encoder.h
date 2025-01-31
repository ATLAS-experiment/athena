/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*EXTERNAL CODE PORTED FROM YARR MINIMALLY ADAPTED FOR ATHENA,
  including the mutex in addToStream, which is the only externally-called
  function that accesses the class resources.*/


/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 02/2024
* Description: ITkPixv2 encoding
*/

#ifndef ITKPIXV2ENCODER_H
#define ITKPIXV2ENCODER_H

#include "ITkPixEncoder.h"
#include <mutex>

class ITkPixV2Encoder : public ITkPixEncoder {
    
    public:
        
        void endStream() const;
        
        void addToStream(const HitMap& hitMap, bool last = false) const;

};

#endif
