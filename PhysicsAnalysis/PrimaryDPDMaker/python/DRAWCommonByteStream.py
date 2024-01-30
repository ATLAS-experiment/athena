# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging
import os

def DRAWCommonByteStreamCfg(flags,
                            formatName,
                            filename):
    '''Common config fragment for setting up 
       ByteStream for DRAW formats'''
    if os.access(filename,os.F_OK):
        msg=logging.getLogger('DRAWByteStreamCfg')
        msg.warning(f"Deleting pre-existing DRAW file {filename}")
        os.remove(filename)


    # Output configuration
    outCA = ComponentAccumulator(
        CompFactory.AthSequencer(
            name='AthOutSeq',
            StopOverride=True))

    bsesoSvc = CompFactory.ByteStreamEventStorageOutputSvc(
        name="BSEventStorageOutputSvc"+formatName,
        MaxFileMB=15000,
        MaxFileNE=15000000,
        OutputDirectory='./',
        StreamType='',
        StreamName='Stream'+formatName,
        SimpleFileName=filename)
    outCA.addService(bsesoSvc)

    bsIS = CompFactory.ByteStreamEventStorageInputSvc('ByteStreamInputSvc')
    outCA.addService(bsIS)

    bsCopyTool = CompFactory.ByteStreamOutputStreamCopyTool(
        ByteStreamOutputSvc=bsesoSvc,
        ByteStreamInputSvc=bsIS)

    bsCnvSvc = CompFactory.ByteStreamCnvSvc(
        ByteStreamOutputSvcList=[bsesoSvc.getName()])
    outCA.addService(bsCnvSvc)

    outCA.addEventAlgo(CompFactory.AthenaOutputStream(
        name='BSOutputStreamAlg'+formatName,
        WritingTool=bsCopyTool,
        EvtConversionSvc=bsCnvSvc.name,
        RequireAlgs=[formatName+'Kernel'],
        ExtraInputs={('xAOD::EventInfo', 'StoreGateSvc+EventInfo')}),
        domain='IO', primary=True)

    from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
    outCA.merge(IOVDbSvcCfg(flags))
    outCA.merge(MetaDataSvcCfg(flags,
                               ["IOVDbMetaDataTool", "ByteStreamMetadataTool"]))

    return outCA
