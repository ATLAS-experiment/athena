# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def getEnvelopeMap(flags):
    # Map of volume name to output collection name
    envelopeMap = dict()

    # TODO - implement CTB config
    # if flags.TestBeam.LArFarUpstreamMaterial:
    #     envelopeMap['LARFARUPSTREAMMATERIAL::LARFARUPSTREAMMATERIAL'] = 'LArFarUpstreamMaterialExitLayer'
    from AthenaConfiguration.Enums import BeamType
    if flags.Beam.Type is BeamType.Cosmics:
        ## First filter volume
        if "TRT_Barrel" in flags.Sim.CosmicFilterVolumeNames:
            envelopeMap['TRT::BarrelOuterSupport'] = 'TRTBarrelEntryLayer'
        elif "TRT_EC" in flags.Sim.CosmicFilterVolumeNames:
            envelopeMap['TRT::WheelA'] = 'TRTECAEntryLayer'
            envelopeMap['TRT::WheelB'] = 'TRTECBEntryLayer'
        elif "TRT_EC" in flags.Sim.CosmicFilterVolumeNames:
            envelopeMap['SCT::ThShieldOuterCly'] = 'SCTBarrelEntryLayer'# could be ThShieldInnerCly or Cyl..
        elif "Pixel" in flags.Sim.CosmicFilterVolumeNames:
            envelopeMap['Pixel::Pixel'] = 'PixelEntryLayer'
    if not flags.Sim.ISFRun:
        if flags.Detector.GeometryID:
            envelopeMap['IDET::IDET'] = 'CaloEntryLayer'
        if flags.Detector.GeometryITk:
            envelopeMap['ITK::ITK'] = 'CaloEntryLayer'
        if flags.Detector.GeometryCalo:
            envelopeMap['CALO::CALO'] = 'MuonEntryLayer'
        if flags.Detector.GeometryMuon: #was geometry in old style, should it be?
            envelopeMap['MUONQ02::MUONQ02'] = 'MuonExitLayer'
    return envelopeMap


def MCTruthSteppingActionToolCfg(flags, name='G4UA::MCTruthSteppingActionTool', **kwargs):
    """Retrieve the MCTruthSteppingActionTool"""
    result = ComponentAccumulator()
    kwargs.setdefault("VolumeCollectionMap", getEnvelopeMap(flags))

    result.setPrivateTools( CompFactory.G4UA.MCTruthSteppingActionTool(name, **kwargs) )
    return result
