# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import MadGraphControl.MadGraphUtils
MadGraphControl.MadGraphUtils.MADGRAPH_PDFSETTING={
    'central_pdf': 260800, #NNPDF30_nlo_as_0118_mc
    'pdf_variations':[260800,93300], # NNPDF30_nlo_as_0118_mc PDF4LHC21_40_pdfas
    # changes wrt to non-MC setting: default NNPDF30 and MC NNPDF31 as alternative weight
    'alternative_pdfs':[266000, #NNPDF30_nlo_as_0119
                        265000, #NNPDF30_nlo_as_0117
                        303200, #NNPDF30_nnlo_as_0118_hessian
                        27400, #MSHT20nnlo_as118
                        27100, #MSHT20nlo_as118
                        14000, #CT18NNLO
                        14400, #CT18NLO
                        304400, #NNPDF31_nnlo_as_0118_hessian
                        304200, #NNPDF31_nlo_as_0118_hessian
                        331500, #NNPDF40_nnlo_as_01180_hessian
                        331700, #NNPDF40_nlo_as_01180
                        14200, #CT18ANNLO
                        14300, #CT18XNNLO
                        14100, #CT18ZNNLO
                        13100, #CT14nlo 
                        25200, #MMHT2014nlo68clas118
                        26000, #NNPDF30_nlo_as_0118
                        315800, #NNPDF31_nlo_as_0118_mc
                        ],    
    'scale_variations':[0.5,1.,2.],
}

