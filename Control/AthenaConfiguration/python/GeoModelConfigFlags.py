# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.AutoConfigFlags import GetFileMD, DetDescrInfo
from AthenaConfiguration.Enums import LHCPeriod, ProductionStep, Project

def createGeoModelConfigFlags(analysis=False):
    gcf=AthConfigFlags()

    def __getTrigTag(flags):
        from TriggerJobOpts.TriggerConfigFlags import trigGeoTag
        return trigGeoTag(flags)

    gcf.addFlag("GeoModel.AtlasVersion", lambda flags :
                (__getTrigTag(flags) if flags.Trigger.doLVL1 or flags.Trigger.doHLT else None) or
                GetFileMD(flags.Input.Files).get("GeoAtlas", None), help='ATLAS Geometry version tag')

    # Special handling of analysis releases where we only want AtlasVersion and Run
    if analysis:
        def _deduce_LHCPeriod(prevFlags):
            import logging
            log = logging.getLogger("GeoModelConfigFlags")
            log.info('Deducing LHC Run period from the geometry tag name "%s" as database access is not available in analysis releases', prevFlags.GeoModel.AtlasVersion)
            if not prevFlags.GeoModel.AtlasVersion:
                raise ValueError('No geometry tag specified')

            if prevFlags.GeoModel.AtlasVersion.startswith("ATLAS-R1"):
                period = LHCPeriod.Run1
            elif prevFlags.GeoModel.AtlasVersion.startswith("ATLAS-R2"):
                period = LHCPeriod.Run2
            elif prevFlags.GeoModel.AtlasVersion.startswith("ATLAS-R3"):
                period = LHCPeriod.Run3
            else:
                raise ValueError(f'Can not deduce LHC Run period from "{prevFlags.GeoModel.AtlasVersion}", please set "flags.GeoModel.Run" manually.')

            log.info('Using LHC Run period "%s"', period.value)
            return period

        gcf.addFlag("GeoModel.Run",  # Run deduced from other metadata
                    _deduce_LHCPeriod, type=LHCPeriod, help='LHC Run period')
        return gcf

    def _deduce_LHCPeriod(prevFlags):
        if prevFlags.GeoModel.AtlasVersion:
            return LHCPeriod(DetDescrInfo(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath)['Common']['Run'])

        if prevFlags.Input.isMC:
            raise ValueError('No geometry tag specified')

        if 2022 <= prevFlags.Input.DataYear:
            return LHCPeriod.Run3
        if 2015 <= prevFlags.Input.DataYear <= 2018:
            return LHCPeriod.Run2
        if 2010 <= prevFlags.Input.DataYear <= 2013:
            return LHCPeriod.Run1

        raise RuntimeError('Can not determine LHC period from the data project name')

    gcf.addFlag("GeoModel.Run", _deduce_LHCPeriod, type=LHCPeriod, help='LHC Run period')

    gcf.addFlag('GeoModel.Layout', 'atlas', help='Geometry layout') # replaces global.GeoLayout

    gcf.addFlag("GeoModel.Align.Dynamic",
                lambda prevFlags : prevFlags.GeoModel.Run >= LHCPeriod.Run2 and not prevFlags.Input.isMC, help='Flag for using dynamic alignment')
                # TODO: dynamic alignment is for now enabled by default for data overlay
                # to disable, add 'and prevFlags.Common.ProductionStep not in [ProductionStep.Simulation, ProductionStep.Overlay]'

    def _deduce_LegacyConditionsAccess(prevFlags):
        if prevFlags.Common.Project is Project.AthSimulation:
            return True
        if prevFlags.Common.ProductionStep is not ProductionStep.Simulation:
            return False
        from SimulationConfig.SimEnums import LArParameterization
        if prevFlags.Sim.ISF.Simulator.usesFastCaloSim() or prevFlags.Sim.LArParameterization is LArParameterization.FastCaloSim:
            return False
        return True

    gcf.addFlag("GeoModel.Align.LegacyConditionsAccess", _deduce_LegacyConditionsAccess,
                help='Flag for using the legacy conditions access infrastructure')
                # Mainly for G4 which still loads alignment on initialize

    gcf.addFlag("GeoModel.Type",
                lambda prevFlags : DetDescrInfo(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath)['Common']['GeoType'],
                help='Geometry type in {ITKLoI, ITkLoI-VF, etc...}')

    gcf.addFlag("GeoModel.IBLLayout",
                lambda prevFlags : DetDescrInfo(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath)['Pixel']['IBLlayout'],
                help='IBL layer layout in {"planar", "3D", "noIBL"}')

    gcf.addFlag('GeoModel.EMECStandard',False, help='Flag for activating the EMEC description with standard Geant4 shapes: G4GenericTrap')

    gcf.addFlag('GeoModel.SQLiteDB',False, help='Flag for activating GeoModel initialization from SQLite Geometry DB')

    gcf.addFlag('GeoModel.SQLiteDBFullPath','', help='Override default location of the SQLite Geometry DB')

    gcf.addFlag('GeoModel.IgnoreTagDifference',False, help='Ignore geometry tag difference between the configured value and the value read from the input file metadata')

    return gcf
