/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKGLOBALCHI2ALIGNTOOLS_GLOBALCHI2ALIGNTOOL_H
#define TRKGLOBALCHI2ALIGNTOOLS_GLOBALCHI2ALIGNTOOL_H

#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "TrkAlignInterfaces/IAlignTool.h"
#include "TrkAlignInterfaces/IMatrixTool.h"
#include "TrkAlignInterfaces/IAlignModuleTool.h"

//#include "TMatrixD.h"

#include <string>

/** @file GlobalChi2AlignTool.h
    @class GlobalChi2AlignTool

    @brief Based on SiGlobalChi2Algs and TRT_AlignAlgs.  
    AlgTool used to calculate and store first- and second-derivatives for 
    global chi2 alignment of any set of detector modules defined using 
    AlignModule.  Also provides methods for solving for the final alignment 
    parameters.
    
    @author Robert Harrington <roberth@bu.edu>
    @date 1/5/08
*/

class TFile;
class TTree;

namespace Trk {
  class Track;

  class GlobalChi2AlignTool : virtual public IAlignTool, public AthAlgTool {    
  public:
    
    /** Constructor */
    GlobalChi2AlignTool(const std::string& type, const std::string& name,
	       const IInterface* parent);
    
    /** Virtual destructor */
    virtual ~GlobalChi2AlignTool();
    
    /** initialize */
    virtual StatusCode initialize();
    
    /** finalize */
    virtual StatusCode finalize();

    /** sets up matrix */
    virtual StatusCode firstEventInitialize();

    /** solves for alignment parameters */
    virtual StatusCode solve();

    /** accumulates information from an AlignTrack */
    bool accumulate(AlignTrack* alignTrack);

    /** accumulates information from binary files */
    bool accumulateFromFiles();

    /** returns total number of accumulated tracks */
    unsigned int nAccumulatedTracks() { return m_ntracks; };

    /** set ntuple */
    void setNtuple(TFile* ntuple) { m_ntuple=ntuple; }

    /** sets the output stream for the logfile */
    void setLogStream(std::ostream * os);

    /** writes tree to ntuple */
    StatusCode fillNtuple();

  private:

    double getMaterialOnTrack(const Trk::Track* track);
  
    /** Pointer to MatrixTool, used to write to and solve matrix*/
    ToolHandle<Trk::IMatrixTool> m_matrixTool{this, "MatrixTool", "Trk::MatrixTool",
      "tool for storing and inverting matrix"};

    /** Pointer to AlignModuleTool*/
    ToolHandle<Trk::IAlignModuleTool> m_alignModuleTool{this, "AlignModuleTool",
      "Trk::AlignModuleTool/AlignModuleTool"};
    
    std::string m_pathbin;            //!< path binary files (in/out)
    std::string m_pathtxt;            //!< path ascii files (in/out)
    std::string m_prefixName;         //!< prefix string to filenames

    unsigned int m_ntracks = 0; //!< number of accumulated tracks
    unsigned int m_nmeas = 0;   //!< number of accumulated measurements
    unsigned int m_nhits = 0;   //!< number of accumulated hits
    double m_chi2 = 0.;          //!< total chi2
    unsigned int m_nDoF = 0;    //!< number of degrees of freedom

    Gaudi::Property<double> m_secondDerivativeCut{this, "SecondDerivativeCut", -1e5};
    Gaudi::Property<bool>   m_doTree{this, "DoTree", false};
    Gaudi::Property<bool>   m_writeActualSecDeriv{this, "WriteActualSecDeriv", false};
    Gaudi::Property<bool>   m_storeLocalDerivOnly{this, "StoreLocalDerivOnly", false};

    static constexpr int MAXNCHAMBERS=50;
    static constexpr int MAXNINDICES =50*6;

    /** output ntuple */
    TFile*   m_ntuple = nullptr;
    TTree*   m_tree = nullptr;
    int      m_run{};
    int      m_event{};
    double   m_materialOnTrack{};
    double   m_momentum{};
    int      m_nChambers = 0;
    int*     m_chamberIds = new int[MAXNCHAMBERS];
    int      m_nMatrixIndices = 0;
    int*     m_matrixIndices = new int[MAXNINDICES];
    int      m_nSecndDeriv = 0;
    double*  m_secndDeriv = new double[MAXNINDICES*MAXNINDICES];
    double*  m_firstDeriv = new double[MAXNINDICES];
    double*  m_actualSecndDeriv = new double[MAXNINDICES];
    double   m_eta{};
    double   m_phi{};
    double   m_perigee_x{};
    double   m_perigee_y{};
    double   m_perigee_z{};
    int      m_trackInfo{};
    int      m_bremFit{};
    int      m_bremFitSuccessful{};
    int      m_straightTrack{};
    int      m_slimmedTrack{};
    int      m_hardScatterOrKink{};
    int      m_lowPtTrack{};
              
    bool     m_fromFiles = false;

  }; // end of class

} // End of namespace 

#endif // TRKALIGNGENTOOLS_MATRIXTOOL_H



