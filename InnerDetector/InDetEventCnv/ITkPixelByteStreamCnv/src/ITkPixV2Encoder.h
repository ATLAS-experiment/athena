/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 02/2024
* Description: ITkPixv2 encoding
*/

#ifndef ITKPIXV2ENCODER_H
#define ITKPIXV2ENCODER_H

#include "ITkPixEncoder.h"

class ITkPixV2Encoder : public ITkPixEncoder {
    
    public:
        
        void endStream();
        
        void addToStream(const HitMap& hitMap, bool last = false);
};

#endif
