# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from functools import cache
from typing import Any, cast
from collections.abc import Iterable
from pycrest.api.crest_api import CrestApi, HTTPResponse, IovSetDto, TagMetaSetDto

import json
from pprint import pprint
from datetime import datetime as dt

from AthenaCommon.Logging import logging
log = logging.getLogger('TriggerCrestUtil.py')

class TriggerCrestUtil:

    # the mapping from triggerdb or triggerdb-alias to crest connection
    # See https://its.cern.ch/jira/browse/ATR-32030    
    dbname_crestconn_mapping: dict[str, str] = {
        # schema name mapping
        "ATLAS_CONF_TRIGGER_RUN3": "CONF_DATA_RUN3",
        "ATLAS_CONF_TRIGGER_MC_RUN3": "CONF_MC_RUN3",
        "ATLAS_CONF_TRIGGER_REPR_RUN3": "CONF_REPR_RUN3",
        "ATLAS_CONF_TRIGGER_RUN4": "CONF_DATA_RUN4",
        "ATLAS_CONF_TRIGGER_MC_RUN4": "CONF_MC_RUN4",
        "ATLAS_CONF_TRIGGER_REPR_RUN4": "CONF_REPR_RUN4",
        "ATLAS_CONF_TRIGGER_LS3_DEV": "CONF_DEV_LS3",
        "ATLAS_CONF_TRIGGER_LS3_L0": "CONF_DEV_L0",
        "ATLAS_CONF_TRIGGER_RUN2_NF": "CONF_DATA_RUN2",
        # alias mapping
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

    crestconn_dbname_mapping: dict[str, str] = {
        # schema name mapping
        "CONF_DATA_RUN3": "ATLAS_CONF_TRIGGER_RUN3",
        "CONF_MC_RUN3": "ATLAS_CONF_TRIGGER_MC_RUN3",
        "CONF_REPR_RUN3": "ATLAS_CONF_TRIGGER_REPR_RUN3",
        "CONF_DATA_RUN4": "ATLAS_CONF_TRIGGER_RUN4",
        "CONF_MC_RUN4" : "ATLAS_CONF_TRIGGER_MC_RUN4",
        "CONF_REPR_RUN4" : "ATLAS_CONF_TRIGGER_REPR_RUN4",
        "CONF_DEV_LS3" : "ATLAS_CONF_TRIGGER_LS3_DEV",
        "CONF_DEV_L0" : "ATLAS_CONF_TRIGGER_LS3_L0",
        "CONF_DATA_RUN2" : "ATLAS_CONF_TRIGGER_RUN2_NF"
    }
    
    @staticmethod
    def allCrestConnections() -> list[str]:
        """list of all known crest connections

        Returns:
            list[str]: list of crest connection names
        """
        return list(TriggerCrestUtil.dbname_crestconn_mapping.values())

    @staticmethod
    def getCrestConnection(dbname: str) -> str | None:
        """maps from triggerdb schema or triggerdb-alias to crest connection
        See https://its.cern.ch/jira/browse/ATR-32030
        
        If the input dbname is already a crest connection name, it is returned as is.

        Args:
            dbname (str): triggerdb name or alias or crest connection name.
        Returns:
            str | None: crest connection name or None if not found.
        """
        if dbname in TriggerCrestUtil.dbname_crestconn_mapping.values():
            return dbname
        return TriggerCrestUtil.dbname_crestconn_mapping.get(dbname, None)

    @cache
    @staticmethod
    def getDBNameMapping() -> dict[str, str]:
        return TriggerCrestUtil.dbname_crestconn_mapping

    @staticmethod
    def getCrestApi(server: str) -> CrestApi:
        """ Crest API object

        Args:
            server (str): crest server name.

        Returns:
            CrestApi: Crest API instance.
        """
        return CrestApi(host=server)

    @staticmethod
    def getConditionsForTimestamp(tag: str, *, timestamp: int, server: str = "", api: CrestApi | None = None,
                                  get_time_type: bool = False) -> dict[str, dict[str, Any]]:
        """ Conditions data for tag at a specific timestamp

        Conditions data is returned as a dict with 'since' and 'payload' keys.
        The 'payload' itself is a dict of channel:'data dicts'. The 'data dict' has attribute names as
        keys and the corresponding values. If get_time_type is True, also 'time_type' key is added to 
        each IOV dict and the since is further expanded into 'since_run' and 'since_lb' (for run/lb-based 
        timestamps) or 'since_formatted' (for time-based timestamps).

        Args:
            tag (str): The tag name
            timestamp (int): The timestamp to query (either run<32+lb or time in nanoseconds since 1.1.1970)
            server (str, optional): Crest server name. Only needed if api is not provided. Defaults to "".
            api (_type_, optional): Crest API instance. Defaults to None.
            get_time_type (bool, optional): Whether to retrieve the time type. Defaults to False.

        Raises:
            RuntimeError: If both server and api are missing

        Returns:
            dict[str, dict[str, any]]: Conditions data
        """
        if server=="" and api is None:
            log.error("Need either crest server name or crest api specified for accessing Crest")
            raise RuntimeError("Crest access information missing")
        if api is None:
            api = CrestApi(host=server)
        # get the payload specification
        attr_list, _ = TriggerCrestUtil._get_payload_spec(tag, api)
        # get the IOVs in the given range
        iov = TriggerCrestUtil._get_iov_for_timestamp(tag, timestamp=timestamp, api=api)

        if get_time_type:
            time_type = TriggerCrestUtil.getTagTimeType(tag, api=api)

        payload_for_iov = {}
        payload_hash: str = cast(list, iov['resources'])[0]['payload_hash']
        since: int = cast(list, iov['resources'])[0]['since']
        payload = TriggerCrestUtil.getPayloadFromHash(payload_hash, api=api)
        for channel, data in payload.items():
            payload_for_iov[channel] = dict(zip(attr_list, data))

        result: dict[str, Any] = {
            'since': since,
            'payload': payload_for_iov
        }
        if get_time_type:
            TriggerCrestUtil._update_with_time_type(result, time_type, since)
        return result


    @staticmethod
    def getConditionsInRange(tag: str, *, since: int, until: int, server: str = "", api: CrestApi | None = None,
                             get_time_type: bool = False) -> list[dict[str, dict[str, Any]]]:
        """ Conditions data for tag in given range

        List goes over the IOVs found. For each IOV, there is a dict with 'since' and 'payload' keys. 
        The 'payload' itself is a dict of channel:'data dicts'. The 'data dict' has attribute names as
        keys and the corresponding values. If get_time_type is True, also 'time_type' key is added to 
        each IOV dict and the since is further expanded into 'since_run' and 'since_lb' (for run/lb-based 
        timestamps) or 'since_formatted' (for time-based timestamps).

        Args:
            tag (str): tag name
            since (int): start of the range (either run<32+lb or time in nanoseconds since 1.1.1970, inclusive)
            until (int): end of the range (either run<32+lb or time in nanoseconds since 1.1.1970, exclusive)
            server (str, optional): crest server name. Only needed if api is not provided. Defaults to "".
            api (CrestApi, optional): Crest API instance. Defaults to None.
            get_time_type (bool, optional): whether to retrieve the time type. Defaults to False.

        Raises:
            RuntimeError: if both server and api are missing

        Returns:
            list[dict[str, dict[str, any]]]: conditions data
        """
        if server=="" and api is None:
            log.error("Need either crest server name or crest api specified for accessing Crest")
            raise RuntimeError("Crest access information missing")
        if api is None:
            api = CrestApi(host=server)
        # get the payload specification
        attr_list, type_dict = TriggerCrestUtil._get_payload_spec(tag, api)
        # get the IOVs in the given range
        all_iovs = TriggerCrestUtil._get_iovs_range(since=since, until=until, api=api, tag=tag)

        if get_time_type:
            time_type = TriggerCrestUtil.getTagTimeType(tag, api=api)

        result: list[dict[str, dict[str, Any]]] = []
        for iov in cast(Iterable, all_iovs['resources']):
            payload_for_iov = {}
            payload_hash = iov['payload_hash']
            since = iov['since']
            payload = TriggerCrestUtil.getPayloadFromHash(payload_hash, api=api)
            for channel, data in payload.items():
                payload_for_iov[channel] = dict(zip(attr_list, data))
            entry = {'since': since, 'payload': payload_for_iov}
            if get_time_type:
                TriggerCrestUtil._update_with_time_type(entry, time_type, since)
            result += [entry]
        return result

    @staticmethod
    def getTagTimeType(tag: str, *, server: str = "", api: CrestApi | None = None) -> str:
        """ time type of the tag, either 'run-lumi' or 'time'

        Args:
            tag (str): tag name
            server (str, optional): crest server name. Only needed if api is not provided. Defaults to "".
            api (CrestApi, optional): Crest API instance. Defaults to None.

        Raises:
            RuntimeError: if both server and api are missing

        Returns:
            str: time type of the tag ['run-lumi' or 'time']
        """
        if server=="" and api is None:
            log.error("Need either crest_server name or crest api for retrieving the payload")
            raise RuntimeError("Crest access information missing")
        if api is None:
            api = CrestApi(host=server)
        tag_info = api.find_tag(tag)
        time_type: str = tag_info['time_type']
        return time_type

    @staticmethod
    def getAttribs(tag: str, *, server: str = "", api: CrestApi | None = None) -> list[str]:
        """ list of attributes for this tag

        Args:
            tag (str): tag name
            server (str, optional): crest server name. Only needed if api is not provided. Defaults to "".
            api (CrestApi, optional): Crest API instance. Defaults to None.

        Raises:
            RuntimeError: if both server and api are missing

        Returns:
            dict: list of attributes
        """
        if server=="" and api is None:
            log.error("Need either crest_server name or crest api for retrieving the payload")
            raise RuntimeError("Crest access information missing")
        if api is None:
            api = CrestApi(host=server)
        attr_list, _ = TriggerCrestUtil._get_payload_spec(tag, api)
        return attr_list

    @staticmethod
    def getPayloadFromHash(payload_hash: str, *, server: str = "", api: CrestApi | None = None) -> dict:
        """ Get payload from hash

        Args:
            payload_hash (str): hash of the payload
            server (str, optional): crest server name. Defaults to "".
            api (CrestApi, optional): Crest API instance. Defaults to None.

        Raises:
            RuntimeError: if both server and api are missing

        Returns:
            dict: payload retrieved from the hash
        """
        if server == "" and api is None:
            log.error("Need either crest_server name or crest api for retrieving the payload")
            raise RuntimeError("Crest access information missing")
        useWorkAround = True
        if useWorkAround:
            if server == "":
                server = api._host  # type: ignore
            return TriggerCrestUtil._get_payload_workaround(payload_hash, server)
        else:
            if api is None:
                api = CrestApi(host=server)
            return TriggerCrestUtil._get_payload(payload_hash, api)

    # Specific utility functions for trigger configuration retrieval
    @staticmethod
    def getEORParams(run: int, *, server: str = "", api: CrestApi | None = None) -> dict[str, Any] | None:
        if api is None:
            api = CrestApi(host=server)
        start_of_run = (run << 32)
        cond_data = TriggerCrestUtil.getConditionsForTimestamp("TDAQRunCtrlEOR-HEAD", timestamp=start_of_run, api=api, get_time_type=False)
        if not cond_data:
            log.error("No EOR params found for run %s", run)
            return None
        return cond_data['payload']['0']

    @staticmethod
    def getHLTPrescaleKeys(run: int, *, server: str = "", api: CrestApi | None = None) -> list[dict[str, dict[str, Any]]]:  # -> list[dict[str, dict[str, Any]]]:
        if api is None:
            api = CrestApi(host=server)
        run_start = (run << 32)
        run_end = ((run + 1) << 32) - 1
        cond = TriggerCrestUtil.getConditionsInRange("TRIGGERHLTPrescaleKey-HEAD", since=run_start, until=run_end, api=api, get_time_type=True)
        for entry in cond:
            entry.update(entry.pop('payload')['0'])
            entry['key'] = entry['HltPrescaleKey']
        # filter to only those starting in this run (since CREST IOVs are open-ended, we otherwise might get IOVs from previous runs)
        # since the first IOV of a run always starts at lb=0, this should only be the case for future runs and return an empty list in that case
        cond: list[dict[str, dict[str, Any]]] = [entry for entry in cond if entry['since_run']==run]
        return cond

    @staticmethod
    def getHLTPrescaleKey(run: int, lb: int, *, server: str = "", api: CrestApi | None = None) -> int | None:
        if api is None:
            api = CrestApi(host=server)
        run_lb = (run << 32) + lb
        cond_entry: dict[str, Any] = TriggerCrestUtil.getConditionsForTimestamp("TRIGGERHLTPrescaleKey-HEAD", timestamp=run_lb, api=api, get_time_type=True)
        if cond_entry['since_run'] < run: # CREST returned an IOV from a previous run (all IOVs are open-ended in CREST), no prescale key for this run
            return None
        return cond_entry['payload']['0']['HltPrescaleKey']

    @staticmethod
    def getL1PrescaleKeys(run: int, *, server: str = "", api: CrestApi | None = None) -> list[dict[str, dict[str, Any]]]:
        if api is None:
            api = CrestApi(host=server)
        run_start = (run << 32)
        run_end = ((run + 1) << 32) - 1
        cond = TriggerCrestUtil.getConditionsInRange("TRIGGERLVL1Lvl1ConfigKey-HEAD", since=run_start, until=run_end, api=api, get_time_type=True)
        for entry in cond:
            entry.update(entry.pop('payload')['0'])
            entry['key'] = entry['Lvl1PrescaleConfigurationKey']
        # filter to only those starting in this run (since CREST IOVs are open-ended, we otherwise might get IOVs from previous runs)
        # since the first IOV of a run always starts at lb=0, this should only be the case for future runs and return an empty list in that case
        cond: list[dict[str, dict[str, Any]]] = [entry for entry in cond if entry['since_run']==run]
        return cond

    @staticmethod
    def getL1PrescaleKey(run: int, lb: int, *, server: str = "", api: CrestApi | None = None) -> int | None:
        if api is None:
            api = CrestApi(host=server)
        run_lb = (run << 32) + lb
        cond_entry: dict[str, Any] = TriggerCrestUtil.getConditionsForTimestamp("TRIGGERLVL1Lvl1ConfigKey-HEAD", timestamp=run_lb, api=api, get_time_type=True)
        if cond_entry['since_run'] < run: # CREST returned an IOV from a previous run (IOVs are open-ended in CREST), no prescale key for this run
            return None
        return cond_entry['payload']['0']['Lvl1PrescaleConfigurationKey']

    @staticmethod
    def getBunchGroupKeys(run: int, *, server: str = "", api: CrestApi | None = None) -> list[dict[str, dict[str, Any]]]:
        if api is None:
            api = CrestApi(host=server)
        run_start = (run << 32)
        run_end = ((run+1) << 32)-1
        cond = TriggerCrestUtil.getConditionsInRange("TRIGGERLVL1BunchGroupKey-HEAD", since=run_start, until=run_end, api=api, get_time_type=True)
        for entry in cond:
            entry.update(entry.pop('payload')['0'])
            entry['key'] = entry['Lvl1BunchGroupConfigurationKey']
        # filter to only those starting in this run (since CREST IOVs are open-ended, we otherwise might get IOVs from previous runs)
        # since the first IOV of a run always starts at lb=0, this should only be the case for future runs and return an empty list in that case
        cond: list[dict[str, dict[str, Any]]] = [entry for entry in cond if entry['since_run']==run]
        return cond

    @staticmethod
    def getBunchGroupKey(run: int, lb: int, *, server: str = "", api: CrestApi | None = None) -> int | None:
        if api is None:
            api = CrestApi(host=server)
        run_lb = (run << 32) + lb
        cond_entry: dict[str, Any] = TriggerCrestUtil.getConditionsForTimestamp("TRIGGERLVL1BunchGroupKey-HEAD", timestamp=run_lb, api=api, get_time_type=True)
        if cond_entry['since_run'] < run: # CREST returned an IOV from a previous run (all IOVs are open-ended in CREST), no prescale key for this run
            return None
        return cond_entry['payload']['0']['Lvl1BunchGroupConfigurationKey']

    @staticmethod
    def getMenuConfigKey(run: int, *, server: str = "", api: CrestApi | None = None) -> dict[str, Any] | None:
        # helper function to turn the payload of the TRIGGERHLTHltConfigKeys into a dictionary
        def _parse_info(run, cond):
            # Format for ConfigSource (see ATR-21550):
            #   Run-4: currently like Run-3
            #   Run-3: TRIGGERDB_RUN3;22.0.101;Athena,22.0.101 --extra_args ...
            #   Run-2: TRIGGERDBR2R,21.1.24,AthenaP1
            release = 'unknown'
            if run > 379000:  # Run-3
                confsrc = cond['ConfigSource'].split(';')
                dbalias = confsrc[0]
                if len(confsrc) > 2:
                    release = confsrc[2].split(',')[0] + f",{confsrc[1]}"
            else:  # Run-2
                confsrc = cond['ConfigSource'].split(',', maxsplit=1)
                dbalias = confsrc[0]
                if len(confsrc) > 2:
                    release = f"{confsrc[2]},{confsrc[1]}"
            return {
                "SMK": cond['MasterConfigurationKey'],
                "HLTPSK": cond['HltPrescaleConfigurationKey'],
                "DB": dbalias,
                "REL": release
            }

        if api is None:
            api = CrestApi(host=server)

        run_start = (run << 32) + 1
        cond: dict[str, Any] = TriggerCrestUtil.getConditionsForTimestamp("TRIGGERHLTHltConfigKeys-HEAD", timestamp=run_start, api=api, get_time_type=True)
        if cond['since_run'] < run: # CREST returned an IOV from a previous run (all IOVs are open-ended in CREST), no prescale key for this run
            return None
        cond.update(cond.pop('payload')['0'])
        return _parse_info(run, cond)

    @staticmethod
    def getTrigConfKeys(runNumber: int, lumiBlock: int, server: str = "", api: CrestApi | None = None) -> dict[str, Any]:
        if api is None:
            api = CrestApi(host=server)
        bgkey: int | None = TriggerCrestUtil.getBunchGroupKey(runNumber, lumiBlock, api=api)
        l1pskey: int | None = TriggerCrestUtil.getL1PrescaleKey(runNumber, lumiBlock, api=api)
        hltpskey: int | None = TriggerCrestUtil.getHLTPrescaleKey(runNumber, lumiBlock, api=api)
        menucfg: dict[str, Any] | None = TriggerCrestUtil.getMenuConfigKey(runNumber, api=api)
        return {
            "SMK": menucfg['SMK'] if menucfg else None,
            "DB": menucfg['DB'] if menucfg else None,
            "LVL1PSK": l1pskey,
            "HLTPSK": hltpskey,
            "BGSK": bgkey
        }

    # internal helper functions
    @staticmethod
    def _update_with_time_type(result: dict[str, Any], time_type: str, since: int) -> None:
        result['time_type'] = time_type
        if time_type == 'run-lumi':
            result.update({
                'since_run': since >> 32,
                'since_lb': since & 0xFFFFFFFF
            })
        elif time_type == 'time':
            result.update({
                'since_formatted': dt.fromtimestamp(since / 1e9)
            })


    @staticmethod
    def _get_payload(payload_hash, api) -> dict:
        return api.get_payload(hash=payload_hash).decode('utf-8')

    @staticmethod
    def _get_payload_workaround(payload_hash, server) -> dict:
        import requests
        import json
        url = f"{server}/payloads/data"
        params = {
            "format": "BLOB", 
            "hash": payload_hash
        }
        preq = requests.Request(method='GET', url=url, params=params).prepare()
        with requests.Session() as session:
            try:
                resp = session.send(preq)
            except requests.ConnectionError as exc:
                raise RuntimeError(f"Could not connect to CREST server {server}") from exc

            if resp.status_code != 200:
                raise RuntimeError(f"Query {payload_hash} to crest failed with status code {resp.status_code}")

        return json.loads(resp.content)

    @staticmethod
    def _get_iovs_for_run(tag: str, *, run: int, api: CrestApi):
        """Helper to retrieve all IOVs for a run"""
        run_start = (run << 32) + 1
        run_end = ((run + 1) << 32) - 1
        return TriggerCrestUtil._get_iovs_range(since=run_start, until=run_end, api=api, tag=tag)

    @staticmethod
    def _get_iov_for_timestamp(tag: str, *, timestamp: int, api: CrestApi) -> IovSetDto | HTTPResponse:
        """Helper to retrieve the IOV for a given timestamp

        Args:
            tag (str): tag name
            timestamp (int): time-stamp in format run<32+lb or time in nanoseconds since 1.1.1970
            api (CrestApi): Crest API instance.

        Returns:
            IovSetDto: set of IOVs (of size 1)
        """
        # the upper limit of the iov-search is exclusive, so we need to add 1
        return api.select_iovs(tag, "0", str(timestamp+1), sort='id.since:DESC', size=1, snapshot=0)  # type: ignore

    @staticmethod
    def _get_iovs_range(*, since: int, until: int, api: CrestApi, tag: str) -> IovSetDto | HTTPResponse:
        """Helper to retrieve the IOV in a given range

        Args:
            tag (str): tag name
            since (int): start of the range (either run<32+lb or time in nanoseconds since 1.1.1970, inclusive)
            until (int): end of the range (either run<32+lb or time in nanoseconds since 1.1.1970, exclusive)
            api (CrestApi): Crest API instance.

        Returns:
            IovSetDto: set of IOVs
        """
        # the upper limit of the iov-search is exclusive, so we need to add 1 to find the iov which includes 'since'
        iovs = api.select_iovs(tag, "0", str(since+1), sort='id.since:DESC', size=1, snapshot=0) # type: ignore
        if cast(int, iovs['size']) < 1:
            raise RuntimeError(f"Did not get an iov which includes the start of run {since}")
        firstiov = cast(list, iovs['resources'])[0]
        all_iovs = api.select_iovs(tag, str(firstiov['since']), str(until), sort='id.since:ASC', snapshot=0) # type: ignore
        return all_iovs

    @staticmethod
    def _get_payload_spec(tag: str, api: CrestApi) -> tuple[list[Any], dict[Any, Any]]:
        """Helper to retrieve the payload spec for a given tag"""
        meta: TagMetaSetDto | HTTPResponse = api.find_tag_meta(tag)
        tag_info = cast(list, meta['resources'])[0]['tag_info']
        tag_info_dict = json.loads(tag_info)
        attr_list = []
        type_dict = {}
        for d in tag_info_dict['payload_spec']:
            attr_list += list(d)
            type_dict.update(d)
        return attr_list, type_dict


if __name__ == "__main__":

    testrun = 435333  # Run-3
    testrunR2 = 360026  # Run-2
    crest_server = "https://crest.cern.ch/api-v5.0"
    api = CrestApi(crest_server)
    
    print(f"run {testrun}:")
    print("\nSuper master key, etc:")
    pprint(TriggerCrestUtil.getMenuConfigKey(testrun, api=api))
    print("\nL1 prescale keys:")
    pprint(TriggerCrestUtil.getL1PrescaleKeys(testrun, api=api))
    pprint(TriggerCrestUtil.getL1PrescaleKey(testrun, 1, api=api))
    print("\nL1 bunchgroup keys:")
    pprint(TriggerCrestUtil.getBunchGroupKeys(testrun, api=api))
    pprint(TriggerCrestUtil.getBunchGroupKey(testrun, 1, api=api))
    print("\nHLT prescale keys:")
    pprint(TriggerCrestUtil.getHLTPrescaleKeys(testrun, api=api))
    pprint(TriggerCrestUtil.getHLTPrescaleKey(testrun, 506, api=api))
    pprint(TriggerCrestUtil.getHLTPrescaleKey(testrun, 507, api=api))
    print("\nEOR params:")
    pprint(TriggerCrestUtil.getEORParams(testrun, api=api),sort_dicts=False)

    print("\nNon existing lb:")
    pprint(TriggerCrestUtil.getHLTPrescaleKey(testrun, 1_000_000, api=api))

    print("\nNon existing run:")
    pprint(TriggerCrestUtil.getHLTPrescaleKey(1_000_000, 1, api=api))
    pprint(TriggerCrestUtil.getHLTPrescaleKeys(1_000_000, api=api))
