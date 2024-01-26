# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
## @file TruthD3PDMaker/python/TruthD3PDObject.py
## @brief xAOD truth event D3PD object
## @author sss
## @date Jul, 2014
##


from D3PDMakerCoreComps.D3PDObject          import make_SGDataVector_D3PDObject
from D3PDMakerConfig.D3PDMakerFlags         import D3PDMakerFlags
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


TruthEventD3PDObject = make_SGDataVector_D3PDObject \
                       ('DataVector<xAOD::TruthEvent_v1>',
                        D3PDMakerFlags.TruthEventSGKey,
                                'mcevt_',
                        'GenEventD3PDObject')

TruthEventD3PDObject.defineBlock (
    0, 'mcevt',
    D3PD.AuxDataFillerTool,
    Vars = [
        #'signal_process_id = signalProcessId',
        #'event_number = eventNumber',
        #'event_scale = eventScale',
        #'alphaQCD',
        #'alphaQED',
        'pdf_id1 = PDGID1',
        'pdf_id2 = PDGID2',
        'pdf_x1 = X1',
        'pdf_x2 = X2',
        #'pdf_scale = Q',
        'pdf1 = XF1',
        'pdf2 = XF2',
        'weight = weights',
        ])
                                            
