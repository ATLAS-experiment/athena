# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @file CaloD3PDMaker/python/LArCollisionTimeD3PDObject.py
# @author scott snyder <snyder@bnl.gov>
# @date Mar, 2010
# @brief D3PD object for LAr collision time variables.
#


from D3PDMakerConfig.D3PDMakerFlags    import D3PDMakerFlags, configFlags
from D3PDMakerCoreComps.D3PDObject     import make_SG_D3PDObject
from AthenaConfiguration.ComponentFactory import CompFactory

D3PD = CompFactory.D3PD

_haveCells = 'AllCalo' in configFlags.Input.Collections

LArCollisionTimeD3PDObject = \
           make_SG_D3PDObject ('LArCollisionTime',
                               D3PDMakerFlags.LArCollisionTimeSGKey,
                               'lar_', 'LArCollisionTimeD3PDObject',
                               default_allowMissing = not _haveCells)

if _haveCells:
    def _larCollTimeAlgHook (c, flags, acc,
                             *args, **kw):
        from TileGeoModel.TileGMConfig import TileGMCfg
        acc.merge (TileGMCfg (flags))
        from LArCellRec.LArCollisionTimeConfig import LArCollisionTimeCfg
        acc.merge (LArCollisionTimeCfg (flags))
        return
    LArCollisionTimeD3PDObject.defineHook (_larCollTimeAlgHook)


    LArCollisionTimeD3PDObject.defineBlock \
                  (0, 'LArCollisionTime',
                   D3PD.LArCollisionTimeFillerTool)

else:
    from AthenaCommon.Logging import logging
    mlog = logging.getLogger( 'LArCollisionTimeD3PDObject' )
    mlog.warning ('No AllCalo cell container; skipping.')
