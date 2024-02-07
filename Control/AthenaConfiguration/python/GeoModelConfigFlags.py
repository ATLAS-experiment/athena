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
                GetFileMD(flags.Input.Files).get("GeoAtlas", None))

    # Special handling of analysis releases where we only want AtlasVersion and Run
    if analysis:
        def _deduct_LHCPeriod(prevFlags):
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
                raise ValueError(f'Can not deduct LHC Run period from "{prevFlags.GeoModel.AtlasVersion}", please set "flags.GeoModel.Run" manually.')

            log.info('Using LHC Run period "%s"', period.value)
            return period

        gcf.addFlag("GeoModel.Run",  # Run deducted from other metadata
                    _deduct_LHCPeriod, type=LHCPeriod)
        return gcf

    def _deduct_LHCPeriod(prevFlags):
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

    gcf.addFlag("GeoModel.Run", _deduct_LHCPeriod, type=LHCPeriod)

    gcf.addFlag('GeoModel.Layout', 'atlas') # replaces global.GeoLayout

    gcf.addFlag("GeoModel.Align.Dynamic",
                lambda prevFlags : prevFlags.GeoModel.Run >= LHCPeriod.Run2 and not prevFlags.Input.isMC)
                # TODO: dynamic alignment is for now enabled by default for data overlay
                # to disable, add 'and prevFlags.Common.ProductionStep not in [ProductionStep.Simulation, ProductionStep.Overlay]'

    gcf.addFlag("GeoModel.Align.LegacyConditionsAccess",
                lambda prevFlags : prevFlags.Common.Project is Project.AthSimulation or prevFlags.Common.ProductionStep is ProductionStep.Simulation)
                # Mainly for G4 which still loads alignment on initialize

    gcf.addFlag("GeoModel.Type",
                lambda prevFlags : DetDescrInfo(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath)['Common']['GeoType'])
                # Geometry type in {ITKLoI, ITkLoI-VF, etc...}

    gcf.addFlag("GeoModel.IBLLayout",
                lambda prevFlags : DetDescrInfo(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath)['Pixel']['IBLlayout'])
                # IBL layer layout  in {"planar", "3D", "noIBL"}

    gcf.addFlag('GeoModel.SQLiteDB',False)
                # Switch for activating GeoModel initialization from an SQLite Geometry DB

    gcf.addFlag('GeoModel.SQLiteDBFullPath','')
                # Override full path to the SQLite Geometry DB. For testing purposes only

    gcf.addFlag('GeoModel.IgnoreTagDifference',False)

    return gcf
