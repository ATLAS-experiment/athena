/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//             Interface for jFEXSmallRJetAlgo - Algorithm for small R jet Algorithm in jFEX
//                              -------------------
//     begin                : 03 11 2020
//     email                : varsiha.sothilingam@cern.ch
//***************************************************************************

#ifndef IjFEXSmallRJetAlgo_H
#define IjFEXSmallRJetAlgo_H

#include "GaudiKernel/IAlgTool.h"
#include "L1CaloFEXSim/jTowerContainer.h"

namespace LVL1{

    static const InterfaceID IID_IjFEXSmallRJetAlgo("LVL1::IjFEXSmallRJetAlgo",1, 0);
     
    class IjFEXSmallRJetAlgo : virtual public IAlgTool{
    public:
      static const InterfaceID& interfaceID ( ) ;
      virtual StatusCode safetyTest() = 0;
      virtual void setup(int inputTable[7][7], int inputTableDisplaced[7][7]) = 0;
      virtual bool isSeedLocalMaxima(int seedThreshold) = 0;
      virtual void buildSeeds() = 0;
      virtual unsigned int getTTowerET(unsigned int TTID ) const = 0;
      virtual unsigned int getSmallClusterET() const = 0;
      virtual unsigned int getSmallETRing() const = 0;
      virtual unsigned int getTTIDcentre() const = 0;
      virtual bool getSRjetSat() const = 0;
      virtual void setFPGAEnergy(const std::unordered_map<int,std::vector<int> >& et_map) = 0;

   private:

    }; 
    inline const InterfaceID& LVL1::IjFEXSmallRJetAlgo::interfaceID()
    {
      return IID_IjFEXSmallRJetAlgo;
    }

}
#endif

