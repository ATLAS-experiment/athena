#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

"""
@file ITkGlobalPrimaryVertexMonAlgCfg.py
@author Leonid Serkin and Per Johansson
@date April 2020
@brief Configuration for Run 3 based on InDetGlobalPrimaryVertexMonTool.cxx
"""

from AthenaConfiguration.ComponentFactory import CompFactory

def HistoITkGlobalPrimaryVertexMonAlgCfg(helper, alg, flags=None):

    # Run 4/ITk: |eta| acceptance grows from 2.5 to 4.0 and Phase II
    # pile-up dramatically increases the per-event vertex / track
    # multiplicity ranges.
    isITk = bool(flags) and flags.Detector.GeometryITk
    m_etaMax = 4.0 if isITk else 3.0
    m_pvNTracksMax = 1000 if isITk else 300

    # this creates a "pvGroup" called "alg" which will put its histograms into the subdirectory "PrimaryVertex"
    pvGroup = helper.addGroup(alg, 'PrimaryVertex')
    pathpv = '/ITkGlobal/PrimaryVertex'

    # begin histogram definitions
    varName = 'm_PvX;pvX' #done
    title = 'Primary vertex: x;x (mm);Events'
    pvGroup.defineHistogram(varName, type='TH1F', path=pathpv, title=title, xbins=500, xmin=-1.5, xmax=0.5)

    varName = 'm_PvY;pvY' #done
    title = 'Primary vertex: y;y (mm);Events'
    pvGroup.defineHistogram(varName, type='TH1F', path=pathpv, title=title, xbins=500, xmin=-1.5, xmax=0.5)

    varName = 'm_PvZ;pvZ' #done
    title = 'Primary vertex: z;z (mm);Events'
    pvGroup.defineHistogram(varName, type='TH1F', path=pathpv, title=title, xbins=100, xmin=-200., xmax=200.)

    # Run 4/ITk: Phase II pileup (mu=200) produces many more reconstructed
    # vertices than Run 1-3, so the original 60/50-bin axes overflow on
    # every event.  Widen to 300 bins covering 0..300 to accommodate the
    # Phase II range while remaining compatible with Run 3 sizes.
    varName = 'm_PvN;pvN' #done
    title = 'Total number of vertices (primary and pile up);Total number of vertices;Events'
    pvGroup.defineHistogram(varName, type='TH1F', path=pathpv, title=title, xbins=300, xmin=-0.5, xmax=299.5)

    varName = 'm_nPriVtx;pvNPriVtx' #done
    title = 'Number of primary vertices;Number of primary vertices;Events'
    pvGroup.defineHistogram(varName, type='TH1F', path=pathpv, title=title, xbins=3, xmin=-0.5, xmax=2.5)

    varName = 'm_nPileupVtx;pvNPileupVtx' #done
    title   = 'Number of pileup vertices;Number of pile up vertices;Events'
    pvGroup.defineHistogram(varName, type='TH1F', path=pathpv, title=title, xbins=300, xmin=-0.5, xmax=299.5)
    
    varName = 'm_PvErrX;pvErrX'  #done
    title   = 'Primary vertex: #sigma_{x}; #sigma_{x} (mm);Events'
    pvGroup.defineHistogram(varName, type='TH1F', path=pathpv, title=title, xbins=200, xmin=0., xmax=0.03)
  
    varName = 'm_PvErrY;pvErrY'  #done
    title   = 'Primary vertex: #sigma_{y}; #sigma_{y} (mm);Events'
    pvGroup.defineHistogram(varName, type='TH1F', path=pathpv, title=title, xbins=200, xmin=0., xmax=0.03)

    varName = 'm_PvErrZ;pvErrZ'   #done
    title   = 'Primary vertex: #sigma_{z}; #sigma_{z} (mm);Events'
    pvGroup.defineHistogram(varName, type='TH1F', path=pathpv, title=title, xbins=100, xmin=0., xmax=0.1)

    varName = 'm_PvChiSqDoF;pvChiSqDof'   #done
    title   = 'Primary vertex: #Chi^{2}/DoF of vertex fit;#Chi^{2}/DoF;Events'
    pvGroup.defineHistogram(varName, type='TH1F', path=pathpv, title=title, xbins=100, xmin=0., xmax=5.)

    # NB: filled from Monitored::Scalar("m_PvNTracks", ...) in the C++
    # algorithm; the Run 3 code accidentally bound this histogram to
    # "m_PvN" which produced (size-1) fills instead of per-vertex track
    # multiplicities.
    varName = 'm_PvNTracks;pvNTracks'  #done
    title = 'Number of tracks in primary vertex;Number of tracks;Events'
    pvGroup.defineHistogram(varName, type='TH1F', path=pathpv, title=title, xbins=200, xmin=0., xmax=m_pvNTracksMax)

    varName = 'm_PvTrackPt;pvTrackPt'  #done
    title   = 'Primary vertex: original track p_{t};p_{t} (GeV);Events'
    pvGroup.defineHistogram(varName, type='TH1F', path=pathpv, title=title, xbins=100, xmin=0., xmax=20.)

    varName = 'm_PvTrackEta;pvTrackEta'  #done
    title   = 'Primary vertex: original track #eta; #eta;Events'
    pvGroup.defineHistogram(varName, type='TH1F', path=pathpv, title=title, xbins=100, xmin=-m_etaMax, xmax=m_etaMax)

# end histograms


def ITkGlobalPrimaryVertexMonAlgCfg(helper, acc,
                                      flags, name="ITkGlobalPrimaryVertexMonAlg",
                                      **kwargs):
    kwargs.setdefault("doEnhancedMonitoring", True)

    from AthenaMonitoring.FilledBunchFilterToolConfig import FilledBunchFilterToolCfg
    from AthenaMonitoring.AtlasReadyFilterConfig import AtlasReadyFilterCfg

    monAlg = helper.addAlgorithm(
        CompFactory.ITkGlobalPrimaryVertexMonAlg, name,
        addFilterTools = [FilledBunchFilterToolCfg(flags), AtlasReadyFilterCfg(flags)],
        **kwargs)

    HistoITkGlobalPrimaryVertexMonAlgCfg(helper, monAlg, flags)
    return
