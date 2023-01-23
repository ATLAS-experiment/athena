/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// TruthNtupleTool.h
//   Header file for TruthNtupleTool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Sebastian.Fleischmann@cern.ch
///////////////////////////////////////////////////////////////////

#ifndef TRK_TRUTHNTUPLETOOL_H
#define TRK_TRUTHNTUPLETOOL_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

#include "TrkValInterfaces/ITruthNtupleTool.h"
#include "TrkValInterfaces/ITrackTruthClassifier.h"
#include <vector>


class TH1D;
class TTree;

namespace Trk {


/** @class TruthNtupleTool
    @brief  Tool to fill basic information about the truth record.

        It is called directly by the ntuple writer but provides information also
        for particles which have failed to be reconstructed. Furthermore it manages
        indices to link to ntuple entries of the associated reconstructed particles.

    @author Sebastian.Fleischmann -at- cern.ch, Wolfgang.Liebig -at- cern.ch
*/

class TruthNtupleTool : virtual public Trk::ITruthNtupleTool, public AthAlgTool {
public:

    // standard AlgToolmethods
    TruthNtupleTool(const std::string&,const std::string&,const IInterface*);
    ~TruthNtupleTool();

    // standard Athena methods
    StatusCode initialize();
    StatusCode finalize();

    virtual StatusCode writeTruthData ( const std::vector< Trk::ValidationTrackTruthData >& truthData );

    virtual unsigned int getNumberOfTreeRecords() const;

    virtual StatusCode initBranches(const std::vector<const Trk::ITrackTruthClassifier*>& classifiers,
                                    bool,
                                    const std::vector<std::string>& trackCollectionNames);

private:

    // jobOptions
    std::string m_ntupleTreeName;       //!< jobOption: Ntuple tree name
    std::string m_ntupleFileName;       //!< jobOption: Ntuple file and dir name
    std::vector<double> m_etaBins;
    bool m_fillJets;            //!< jO: jet filling, set from external call

    ServiceHandle<ITHistSvc> m_histSvc;
    TTree* m_nt; //!< Pointer to the NTuple tree

    unsigned int m_numberOfTreeEntries;

    // ntuple variables
    int         m_runNumber;       //!< run number to which this MC truth particle belongs
    int         m_eventNumber;     //!< event number to which this MC truth particle belongs

    std::vector< std::vector<unsigned int>* >   m_TrackLinkIndex;
    std::vector< std::vector<float>* >          m_mc_prob;
    std::vector<unsigned int>                   m_classifications;
    // Truth information
    float       m_mc_d0;            //!< d0 of MC truth particle's perigee parameters
    float       m_mc_z0;            //!< z of MC truth particle's perigee parameters
    float       m_mc_phi0;          //!< phi of MC truth particle's perigee parameters
    float       m_mc_theta;         //!< theta of MC truth particle's perigee parameters
    float       m_mc_qOverP;        //!< q/p of MC truth particle's perigee parameters
    float       m_mc_qOverPt;       //!< q/pT of MC truth particle's perigee parameters
    float       m_mc_eta;           //!< eta of MC truth particle's perigee parameters

    int         m_mc_particleID;    //!< PDG ID of MC truth particle
    int         m_mc_uniqueID;       //!< MC truth particle's uniqueID
    float       m_mc_energy;        //!< MC truth particle's energy at production vertex
    int         m_mc_jetLinkIndex;  //!< link to jet this particle belongs to (if jet tree is ON)
    float       m_mc_prodR; //!< Rxy of particle's production vertex
    float       m_mc_prodz; //!< z coordinate of particle's production vertex

    // statistics
    std::vector< std::vector<TH1D*> >   m_recoTrackCounts;
    std::vector< std::vector<TH1D*> >   m_truthTrackCounts;

    std::vector<const Trk::ITrackTruthClassifier*> m_trackTruthClassifiers;     //!< the truth classifiers

    SG::ReadHandleKey<xAOD::EventInfo>    m_evt  {this, "EvtInfo", "EventInfo", "EventInfo name"};
};


} // end of namespace

#endif // TRK_TRUTHNTUPLETOOL_H

