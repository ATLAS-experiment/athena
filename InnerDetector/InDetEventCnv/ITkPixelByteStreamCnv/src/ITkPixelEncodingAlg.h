/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXEL_ENCODINGALG_H
#define ITKPIXEL_ENCODINGALG_Hi

#include "AthenaBaseComps/AthReentrantAlgorithm.h"


class ITkPixelEncodingAlg : public AthReentrantAlgorithm 
{
  public:
    virtual StatusCode initialize() override;
    virtual StatusCode execute (const EventContext& ctx) const override;

  private:

};
#endif

