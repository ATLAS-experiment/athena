# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from .afp import AFP
from .lar import LAr
from .lucid import Lucid
from .mdt import MDT
from .rpc import RPC
from .sct import SCT
from .tdq import TDQ
from .tile import Tile
from .trt import TRT
from .tgc import TGC
from .stg import STG
from .mmg import MMG
# Non-detector flags
from .magnets import Magnets
from .global_system import Global
from .trig import Trigger
ALL_SYSTEMS = [AFP, LAr, Lucid, MDT, RPC, SCT, TDQ, Tile, TGC, TRT, Magnets, Global, Trigger, STG, MMG]
SYS_NAMES = ", ".join(map(lambda x: x.__name__, ALL_SYSTEMS))

SYSTEM_MAP = dict((x.__name__, x) for x in ALL_SYSTEMS)
