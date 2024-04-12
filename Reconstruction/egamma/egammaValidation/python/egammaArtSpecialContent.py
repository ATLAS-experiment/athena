# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def egammaArtSpecialContent(flags,cfg):
    from OutputStreamAthenaPool.OutputStreamConfig import outputStreamName
    StreamAOD = cfg.getEventAlgo(outputStreamName("AOD"))
    List = StreamAOD.ItemList
    KeyForExisting = 'CaloCalTopoClustersAux'
    AdditionalEgamma = '.ENG_FRAC_EM'

    newItem = ''
    for e in List:
        if e.find(KeyForExisting) >= 0:
            newItem = e + AdditionalEgamma
    newList = [ e for e in List if e.find(KeyForExisting) < 0 ]
    newList.append(newItem)
    StreamAOD.ItemList = newList
