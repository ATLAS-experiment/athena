/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// EnergyLossMonitor.h
///////////////////////////////////////////////////////////////////

#ifndef TRK_ENERGYLOSSMONITOR_H
#define TRK_ENERGYLOSSMONITOR_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/NTuple.h"

#include "TrkValInterfaces/IEnergyLossMonitor.h"

namespace Trk {

/** @class EnergyLossMonitor

    This validation tool Energy states of partlices.

    @author Sebastian.Fleischmann@cern.ch
*/

  class EnergyLossMonitor : virtual public Trk::IEnergyLossMonitor, public AthAlgTool {
    public:

    /** standard AlgTool constructor / destructor */
    EnergyLossMonitor(const std::string&,const std::string&,const IInterface*);
    ~EnergyLossMonitor(){}

    /** standard Athena methods */
    StatusCode initialize();
    StatusCode finalize();

    /** Initialize State */
    virtual void initializeTrack(double p, 
                                 double E,
                                 double eta,
                                 double phi);

    /** Record a single TrackState */
    virtual void recordTrackState(const Amg::Vector3D& pos, 
                                  const Amg::Vector3D& mom,
                                  double mass);

    /** Finalization State */
    virtual void finalizeTrack();
    
private:    
    /** Ntuple Business */
    ServiceHandle<INTupleSvc>                m_ntupleSvc;
    std::string                              m_outputNtuplePath;
    std::string                              m_outputNtupleDescription;

    /** Ntuple helper */
    int                              m_currentStep;    

    /** Step variables */
    NTuple::Item<long>               m_steps;
    /** Initial variables */
    NTuple::Item<float>              m_initialP;
    NTuple::Item<float>              m_initialE;
    NTuple::Item<float>              m_initialEta;
    NTuple::Item<float>              m_initialPhi;

    NTuple::Array<float>             m_p;
    NTuple::Array<float>             m_E;
    NTuple::Array<float>             m_eta;  
    NTuple::Array<float>             m_phi;
    NTuple::Array<float>             m_hitX;
    NTuple::Array<float>             m_hitY;
    NTuple::Array<float>             m_hitR;
    NTuple::Array<float>             m_hitZ;

};


} // end of namespace

#endif // TRK_TRACKPOSITIONNTUPLEHELPER_H
