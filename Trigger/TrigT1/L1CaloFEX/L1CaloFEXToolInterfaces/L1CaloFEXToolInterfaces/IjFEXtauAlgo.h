/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//             Interface for jFEXtauAlgo - Algorithm for Tau Algorithm in jFEX
//                              -------------------
//     begin                : 18 02 2021
//     email                : Sergi.Rodriguez@cern.ch
//***************************************************************************

#ifndef IjFEXtauAlgo_H
#define IjFEXtauAlgo_H

#include "GaudiKernel/IAlgTool.h"

namespace LVL1 {

static const InterfaceID IID_IjFEXtauAlgo("LVL1::IjFEXtauAlgo",1, 0);

class IjFEXtauAlgo : virtual public IAlgTool {
    public:
        static const InterfaceID& interfaceID ( ) ;
        virtual StatusCode safetyTest() = 0;
        virtual void setup(int seed[3][3]) = 0;
        virtual bool isSeedLocalMaxima() = 0;
        virtual bool isSeedLocalMaxima_fwd(unsigned int TTID) = 0;
        virtual void setFirstEtRing(int First_ETring[]) =0;

        virtual int getClusterEt() const =0;
        virtual int getFirstEtRing() const =0;
        virtual bool getTauSat() const =0;
        virtual void setFPGAEnergy(const std::unordered_map<int,std::vector<int> >& et_map)   =0;

    private:

};
inline const InterfaceID& LVL1::IjFEXtauAlgo::interfaceID()
{
    return IID_IjFEXtauAlgo;
}

}
#endif

