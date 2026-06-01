# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaCommon.Logging import logging
log = logging.getLogger('TriggerCrestUtil.py')

class TriggerCrestUtil:

    @staticmethod
    def getCrestConnection(oracle_db: str) -> str | None:
        """maps from triggerdb or triggerdb-alias to crest connection
        
        See https://its.cern.ch/jira/browse/ATR-32030
        """
        db_mapping: dict[str, str] = {
            "ATLAS_CONF_TRIGGER_RUN3": "CONF_DATA_RUN3",
            "ATLAS_CONF_TRIGGER_MC_RUN3": "CONF_MC_RUN3",
            "ATLAS_CONF_TRIGGER_REPR_RUN3": "CONF_REPR_RUN3",
            "ATLAS_CONF_TRIGGER_RUN4": "CONF_DATA_RUN4",
            "ATLAS_CONF_TRIGGER_MC_RUN4": "CONF_MC_RUN4",
            "ATLAS_CONF_TRIGGER_REPR_RUN4": "CONF_REPR_RUN4",
            "ATLAS_CONF_TRIGGER_LS3_DEV": "CONF_DEV_LS3",
            "ATLAS_CONF_TRIGGER_LS3_L0": "CONF_DEV_L0",
            "ATLAS_CONF_TRIGGER_RUN2_NF": "CONF_DATA_RUN2"
        }
        alias_mapping: dict[str, str] = {
            "TRIGGERDB_RUN3": "CONF_DATA_RUN3",
            "TRIGGERDBREPR_RUN3": "CONF_REPR_RUN3",
            "TRIGGERDBMC_RUN3": "CONF_MC_RUN3",
            "TRIGGERDB_RUN4": "CONF_DATA_RUN4",
            "TRIGGERDBREPR_RUN4": "CONF_REPR_RUN4",
            "TRIGGERDBMC_RUN4": "CONF_MC_RUN4",
            "TRIGGERDBLS3_DEV": "CONF_DEV_LS3",
            "TRIGGERDBLS3_L0": "CONF_DEV_L0",
            "TRIGGERDB_RUN2_NF": "CONF_DATA_RUN2_NF"
        }
        db_mapping.update(alias_mapping)
        return db_mapping.get(oracle_db, None)
