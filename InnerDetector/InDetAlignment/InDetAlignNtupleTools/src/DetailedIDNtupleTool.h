/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETALIGNNTUPLETOOLS_DETAILEDIDNTUPLETOOL_H
#define INDETALIGNNTUPLETOOLS_DETAILEDIDNTUPLETOOL_H

#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkToolInterfaces/ITruthToTrack.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkAlignInterfaces/IFillNtupleTool.h"
#include "TrkAlignInterfaces/IAlignModuleTool.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include  "TrkParameters/TrackParameters.h"


/**
	@Tool for writing information to ntuple for debugging.
	@author Jike Wang <jike.wang@cern.ck>
	@date dd/CLHEP::mm/yyy
*/

class TFile;
class TTree;
class TRT_ID;

namespace Trk
{
  class AlignTrack;
  class Track; 
}

namespace InDet
{
  class DetailedIDNtupleTool : public extends<AthAlgTool, Trk::IFillNtupleTool>
  {
  public:
    using base_class::base_class;
    virtual ~DetailedIDNtupleTool();

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    /** fills track information to ntuple */
    virtual void dumpTrack(const EventContext& ctx, int itrk, const Trk::AlignTrack * alignTrack) override;

    virtual void storeHitmap() override;
    virtual void fillHitmap() override;

    virtual void fillSummary() override;
    virtual void showStatistics() override;

    virtual void setNtuple(TFile* ntuple) override { m_file=ntuple; }
    virtual StatusCode fillNtuple() override;

  private:
    bool retrieveTruthInfo(const Trk::AlignTrack* trk);
    const Trk::TrackParameters* perigeeParameter(const Trk::AlignTrack* track) const;
    void initializeNtuple();

    ToolHandle <Trk::ITrackSummaryTool>         m_trackSumTool
      {this,"TrackSummaryTool","Trk::TrackSummaryTool/TrackSummaryTool",""};    //!< Pointer to track summary tool
    PublicToolHandle <Trk::IAlignModuleTool>    m_alignModuleTool
      {this,"AlignModuleTool","",""}; //!< Pointer to AlignmModuleTool
    PublicToolHandle <Trk::ITruthToTrack>             m_truthToTrack
      {this,"TruthToTrack","Trk::TruthToTrack/InDetTruthToTrack",""};    //!< the truth to track Tool
    PublicToolHandle <Trk::IExtrapolator>             m_extrapolator
      {this,"Extrapolator","Trk::Extrapolator/AtlasExtrapolator",""};    //!< track extrapolator

    Gaudi::Property<bool> m_storeNormalRefittedOnly{this, "StoreNormalRefittedOnly", false}; //!< only store the AlignTrack which is normally refitted
    Gaudi::Property<bool> m_storeConstrainedOnly{this, "StoreConstrainedOnly", true};        //!< onlystore the AlignTrack which is normally refitted
    Gaudi::Property<bool> m_storeTruth{this, "StoreTruth", true};                            //!< retrieve the truth information
    Gaudi::Property<double> m_matchProbability{this, "MatchProbability", 0.8};               //!< the probabililty cut in the truth matching

    TFile * m_file{};
    TTree * m_tree{};

    Gaudi::Property<std::string> m_filename{this, "FileName", "IDAlign.root"}; //!< name of ntuple file
    Gaudi::Property<std::string> m_filepath{this, "FilePath", "./"};           //!< path to ntuple file

    Gaudi::Property<std::string> m_trackCollection{this, "TrackCollection", "Tracks"};               //!
    Gaudi::Property<std::string> m_tracksTruthName{this, "TracksTruthName", "TrackTruthCollection"}; //!

    // Reconstructed variables
    int      m_runNumber = 0;
    int      m_evtNumber = 0;

    // original track information
    double m_original_xvtx = 0.0;
    double m_original_yvtx = 0.0;
    double m_original_zvtx = 0.0;

    double m_original_d0 = 0.0;
    double m_original_z0 = 0.0;
    double m_original_phi0 = 0.0;
    double m_original_theta = 0.0;
    double m_original_qoverp = 0.0;
    double m_original_pt = 0.0;

    double m_original_eta = 0.0;
    double m_original_chi2 = 0.0;
    int    m_original_ndof = 0;
    double m_original_chi2prob = 0.0;

    double m_original_err_d0 = 0.0;
    double m_original_err_z0 = 0.0;
    double m_original_err_phi0 = 0.0;
    double m_original_err_theta = 0.0;
    double m_original_err_qoverp = 0.0;

    // truth informaiton
    double m_truth_d0 = 0.0;
    double m_truth_z0 = 0.0;
    double m_truth_phi0 = 0.0;
    double m_truth_theta = 0.0;
    double m_truth_eta = 0.0;
    double m_truth_pt = 0.0;
    double m_truth_qoverp = 0.0;
    double m_truth_qoverpt = 0.0;
    double m_truth_charge = 0.0;
    double m_truth_prod_x = 0.0;
    double m_truth_prod_y = 0.0;
    double m_truth_prod_z = 0.0;

    // oroginal track wrt reference points
    double m_original_toRef_d0 = 0.0;
    double m_original_toRef_z0 = 0.0;
    double m_original_toRef_phi0 = 0.0;
    double m_original_toRef_theta = 0.0;
    double m_original_toRef_qoverp = 0.0;

    // refitted track wrt reference points
    double m_toRef_d0 = 0.0;
    double m_toRef_z0 = 0.0;
    double m_toRef_phi0 = 0.0;
    double m_toRef_theta = 0.0;
    double m_toRef_qoverp = 0.0;

    // the AlignTrack information
    double m_err_d0 = 0.0;
    double m_err_z0 = 0.0;
    double m_err_phi0 = 0.0;
    double m_err_theta = 0.0;
    double m_err_qoverp = 0.0;

    double m_xvtx = 0.0;
    double m_yvtx = 0.0;
    double m_zvtx = 0.0;
    double m_d0 = 0.0;
    double m_z0 = 0.0;
    double m_phi0 = 0.0;
    double m_theta = 0.0;
    double m_qoverp = 0.0;
    double m_pt = 0.0;
    double m_eta = 0.0;
    double m_chi2 = 0.0;
    int    m_ndof = 0;
    double m_chi2prob = 0.0;

  }; // end class

} // end namespace

#endif // INDETALIGNNTUPLETOOLS_DETAILEDIDNTUPLETOOL_H
