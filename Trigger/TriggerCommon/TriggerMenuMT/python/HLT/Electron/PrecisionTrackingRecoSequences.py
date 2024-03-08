#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TrigInDetConfig.utils import getFlagsForActiveConfig
from TrigInDetConfig.InnerTrackingTrigSequence import InnerTrackingTrigSequence

#logging
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

def precisionTracking(inflags, RoIs, ion=False, variant=''):

    acc = ComponentAccumulator()
    tag = '_ion' if ion is True else ''
    signatureName = 'electronLRT' if variant  else 'electron'
    flags = getFlagsForActiveConfig(inflags, signatureName, log)

    verifier = CompFactory.AthViews.ViewDataVerifier( name = 'VDVInDetPrecision'+variant + tag,
                                                      DataObjects= {( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % RoIs ),
                                                                    ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing' ),
                                                                    ( 'TrackCollection', flags.Tracking.ActiveConfig.trkTracks_FTF )} )

    acc.addEventAlgo(verifier)
    seq = InnerTrackingTrigSequence.create(flags, 
                                           flags.Tracking.ActiveConfig.input_name, 
                                           rois = RoIs, 
                                           inView = verifier.getName())
    acc.merge(seq.sequenceAfterPattern())

    return acc


   
