# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import os
import json
import re
from typing import Any
import six
import xml.etree.ElementTree as ET
import coral

from AthenaCommon.Logging import logging
log = logging.getLogger('TriggerConfigAccessBase.py')

def getFileType(filename):
    filetype = "unknown"
    with open(filename, 'r') as fp:
        config = json.load(fp)
        filetype = config['filetype']
    return filetype

from enum import Enum
class ConfigType(Enum):
    NONE = ("Config", "None", "None")
    L1MENU = ("L1Menu", "l1menu", "L1M")
    HLTMENU = ("HLTMenu", "hltmenu", "HLTM")
    L1PS = ("L1PrescalesSet", "l1prescale", "L1PS")
    HLTPS = ("HLTPrescalesSet", "hltprescale", "HLTPS")
    BGS = ("L1BunchGroupsSet", "bunchgroupset", "BGS")
    HLTJO = ("HLTJobOptions", "joboptions", "JO")
    HLTMON = ("HLTMonitoring", "hltmonitoringsummary", "MGS")
    def __init__(self, basename, filetype, crestkey):
        self.basename = basename
        self.filetype = filetype
        self.crestkey = crestkey
    def __eq__(self, other):
        if isinstance(other,six.string_types):
            return self.filetype == other
        else:
            return self.filetype == other.filetype
    def __ne__(self, other):
        return not self.__eq__(other)

class ConfigLoader:
    """ 
    ConfigLoader derived classes hold the information of the configuration source
    and define the method to load the configuration
    """ 
    def __init__(self,configType):
        self.configType = configType
    def confirmConfigType(self,config):
        """
        checks that the in-file specification of the configuration type matches the expected type
        """
        if config['filetype'] != self.configType:
            raise RuntimeError("Can not load file with filetype '%s' when expecting '%s'" % (config['filetype'], self.configType.filetype))

class ConfigFileLoader(ConfigLoader):
    def __init__(self, configType, filename ):
        super(ConfigFileLoader,self).__init__(configType) 
        self.filename = filename
    def load(self):
        with open(self.filename, 'r') as fp:
            config = json.load(fp)
            self.confirmConfigType(config)
        return config
    def setQuery(self, query):
        pass
    def getWriteFilename(self):
        outfn = os.path.basename(self.filename)
        if outfn.endswith(".json"):
            outfn = outfn.rsplit('.',1)[0]
        return outfn + ".out.json"

class ConfigDirectLoader(ConfigLoader):
    """Class to load from json string"""
    def __init__(self, configType,  jsonString):
        super(ConfigDirectLoader,self).__init__(configType) 
        self.jsonString = jsonString
    def load(self):
        config = json.loads(self.jsonString)
        self.confirmConfigType(config)
        return config
    def setQuery(self, query):
        pass
    def getWriteFilename(self):
        pass

class ConfigDBLoader(ConfigLoader):
    def __init__(self, configType, dbalias, dbkey):
        super().__init__(configType)
        self.dbalias = dbalias
        self.dbkey = dbkey
        self.query = None
        self.schema = None

    def setQuery(self, query):
        """
        query template is a dictionary of queries, identified by schema version, 
        similar to TrigConf::TrigDBMenuLoader::m_hltQueries and TrigConf::TrigDBMenuLoader::m_l1Queries
        """
        self.query = query

    @staticmethod
    def getResolvedFileName(filename, pathenv=""):
        """ looks for file, first absolute, then by resolving envvar pathenv"""
        if os.access(filename,os.R_OK):
            return filename
        pathlist = os.getenv(pathenv,'').split(os.pathsep)
        for path in pathlist:
            f = os.path.join( path, filename )
            if os.access( f, os.R_OK ):
                return f
        raise RuntimeError("Can't read file %s, neither locally nor in %s" % (filename, pathenv) )

    @staticmethod
    def getConnectionParameters(dbalias):
        dblookupFile = ConfigDBLoader.getResolvedFileName("dblookup.xml", "CORAL_DBLOOKUP_PATH")
        dbp = ET.parse(dblookupFile)
        listOfServices = None
        for logSvc in dbp.iter("logicalservice"):
            if logSvc.attrib["name"] != dbalias:
                continue
            listOfServices = [ serv.attrib["name"] for serv in logSvc.iter("service") ]
            if len(listOfServices) == 0:
                raise RuntimeError("DB %s has no services listed in %s" % (dbalias, dblookupFile))
            break
        if listOfServices is None:
            raise RuntimeError("DB %s not available in %s" % (dbalias, dblookupFile))
        
        if "FRONTIER_SERVER" not in os.environ:
            # remove all frontier connnections in the list if the environment FRONTIER_SERVER variable does not exist
            # this speeds up the resolution of the connection specification (dbalias)
            listOfServices = [svc for svc in listOfServices if not svc.startswith("frontier:")]

        # now get the account and pw for oracle connections
        credentials = dict.fromkeys(listOfServices)

        for svc in filter(lambda s : s.startswith("frontier:"), listOfServices):
            credentials[svc] = dict()
            credentials[svc]["user"] = svc
            credentials[svc]["password"] = ""

        try:
            authFile = ConfigDBLoader.getResolvedFileName("authentication.xml", "CORAL_AUTH_PATH")
        except Exception as e:
            log.warning("File authentication.xml is not available! Oracle connection cannot be established. Exception message is: %s",e)
        else:
            for svc in filter(lambda s : s.startswith("oracle:"), listOfServices):
                ap = ET.parse(authFile)
                count = 0
                for con in filter( lambda c: c.attrib["name"]==svc, ap.iter("connection")):
                    credentials[svc] = dict([(par.attrib["name"],par.attrib["value"]) for par in con])
                    count += 1
                if count==0:
                    raise RuntimeError("No credentials found for connection %s from service %s for db %s" % (con,svc,dbalias))
                if count>1:
                    raise RuntimeError("More than 1 connection found in %s for service %s" % (authFile, svc))
        
        return credentials

    @staticmethod
    def getSchema(connStr):
        ''' Read schema from connection string '''
        if connStr.startswith("oracle:"):
            [_, schema] = connStr.split("/")[-2:]
            return schema

        if connStr.startswith("frontier:"):
            import re
            pattern = r"frontier://ATLF/\(\)/(.*)"
            m = re.match(pattern, connStr)
            if not m:
                raise RuntimeError("connection string '%s' doesn't match the pattern '%s'?" % (connStr, pattern))
            (schema, ) = m.groups()
            return schema

        if connStr.startswith("sqlite_file:"):
            raise NotImplementedError("Python-loading of trigger configuration from sqlite has not yet been implemented")

    @staticmethod
    def readSchemaVersion(qdict, session):
        ''' Read schema version form database, based on TrigConf::TrigDBLoader::schemaVersion '''
        try:
            q = "SELECT TS_TAG FROM {schema}.TRIGGER_SCHEMA TS"
            query = ConfigDBLoader.getCoralQuery(session, q.format(**qdict))
            cursor = query.execute()
            cursor.next()

            versionTag = cursor.currentRow()[0].data()

            versionTagPrefix = "Trigger-Run3-Schema-v"
            if not versionTag.startswith(versionTagPrefix):
                raise RuntimeError( "Tag format error: Trigger schema version tag %s does not start with %s", versionTag, versionTagPrefix) 

            vstr = versionTag[len(versionTagPrefix)]

            if not vstr.isdigit():
                raise RuntimeError( "Invalid argument when interpreting the version part %s of schema tag %s is %s", vstr, versionTag, type(vstr))

            log.debug("Found schema version %s", vstr)
            return int(vstr)

        except Exception as e:
            log.warning("Failed to read schema version: %r", e)

    @staticmethod
    def getCoralQuery(session, queryStr, qdict = None):
        ''' Parse output, tables and condition from the query string into coral query object'''
        query = session.nominalSchema().newQuery()

        if qdict is not None:
            queryStr = queryStr.format(**qdict)

        # bind vars
        bindVars = coral.AttributeList()
        bindVarsInQuery = re.findall(r":(\w*)", queryStr)
        if len(bindVarsInQuery) > 0 and qdict is None:
            log.error("Query has bound-variable syntax but no value dictionary is provided. Query: %s", queryStr)
        for k in bindVarsInQuery:
            bindVars.extend(k, "int")
            bindVars[k].setData(qdict[k])

        output = queryStr.split("SELECT")[1].split("FROM")[0]
        for field in output.split(','):
            query.addToOutputList(field)

        log.debug("Conversion for Coral of query: %s", queryStr)

        for table in queryStr.split("FROM")[1].split("WHERE")[0].split(","):
            tableSplit = list(filter(None, table.split(" ")))
            # Schema name is stripped from TableList in Coral query
            query.addToTableList(tableSplit[0].split(".")[1], tableSplit[1])

        if "WHERE" in queryStr:
            cond = queryStr.split("WHERE")[1]
            m = re.match("(.*)(?i: ORDER *BY )(.*)", cond) # check for "order by" clause
            if m:
                where, order = m.groups()
                query.setCondition(where, bindVars)
                query.addToOrderList(order)
            else:
                query.setCondition(cond, bindVars)

        return query

    def getQueryDefinition(self, schemaVersion):
        '''Choose query based on schema version, based on TrigConf::TrigDBLoader::getQueryDefinition '''
        maxDefVersion = 0
        for vkey in self.query.keys():
            if vkey>maxDefVersion and vkey<=schemaVersion:
                maxDefVersion = vkey

        if maxDefVersion == 0:
            raise RuntimeError("No query available for schema version {0}".format(schemaVersion))

        return self.query[maxDefVersion]

    def load(self):
        credentials: dict[str,Any] = ConfigDBLoader.getConnectionParameters(self.dbalias)

        if not credentials:
            log.error("No TriggerDB connections found for %s", self.dbalias)
            raise RuntimeError(f"No TriggerDB connections found for {self.dbalias}")

        svc = coral.ConnectionService() 
        svcconfig = svc.configuration()
        svcconfig.disablePoolAutomaticCleanUp()
        svcconfig.setConnectionTimeOut(0)

        failureMode = 0
        for credential in credentials:
            log.debug("Trying credentials %s",credential)

            try: 
                session = svc.connect(credential, coral.access_ReadOnly)
            except Exception as e:
                log.warning("Failed to establish connection: %s",e)
                failureMode = max(1, failureMode)
                continue

            # Check that the FRONTIER_SERVER is set properly, if not reduce the retrial period and time out values
            if 'FRONTIER_SERVER' in os.environ and os.environ['FRONTIER_SERVER']:
                svcconfig.setConnectionRetrialPeriod(300)
                svcconfig.setConnectionRetrialTimeOut(3600)
            else:
                svcconfig.setConnectionRetrialPeriod(1)
                svcconfig.setConnectionRetrialTimeOut(1)

            try:
                session.transaction().start(True) # readOnly
                self.schema = ConfigDBLoader.getSchema(credential)
                qdict = { "schema" : self.schema, "dbkey" : self.dbkey }
                
                # Choose query based on schema
                schemaVersion = ConfigDBLoader.readSchemaVersion(qdict, session)
                qstr = self.getQueryDefinition(schemaVersion)
                # execute coral query
                query = ConfigDBLoader.getCoralQuery(session, qstr, qdict)
                cursor = query.execute()

            except Exception as e:
                log.warning(f"DB query on {credential} failed to execute.")
                log.warning("Exception message: %r", e)
                failureMode = max(2, failureMode)
                continue # to next source

            # Read query result
            if not cursor.next():
                # empty result
                log.warning(f"DB query on {credential} returned empty result, likely due to non-existing key {self.dbkey}")
                failureMode = 3
                continue # to next source

            configblob = cursor.currentRow()[0].data()
            if type(configblob) is not str:
                configblob = configblob.readline()
            config = json.loads(configblob)
            session.transaction().commit()
            
            self.confirmConfigType(config)
            return config

        if failureMode == 1:
            log.error("TriggerDB query: could not connect to any source for %s", self.configType.basename)
            log.error("Considered sources: %s", ", ".join(credentials))
            raise RuntimeError("TriggerDB query: could not connect to any source", self.configType.basename)
        if failureMode == 2:
            log.error("Query failed due to wrong definition for %s", self.configType.basename)
            log.error("DB query was: %s", qstr.format(**qdict))
            raise RuntimeError("Query failed due to wrong definition", self.configType.basename)
        elif failureMode == 3:
            log.error("DB key %s does not exist for %s", self.dbkey, self.configType.basename)
            raise KeyError("DB key does not exist", self.dbkey, self.configType.basename)
        else:
            raise RuntimeError("Query failed for unknown reason")

    # proposed filename when writing config to file
    def getWriteFilename(self):
        return "{basename}_{schema}_{dbkey}.json".format(basename = self.configType.basename, schema = self.schema, dbkey = self.dbkey)

class ConfigCrestLoader(ConfigLoader):
    def __init__(self, *, configType, dbalias, dbkey, crestServer):
        super().__init__(configType)
        self.crestServer = crestServer
        self.dbalias = dbalias
        self.dbkey = dbkey
        self.schema = ""

    def setQuery(self, query):
        """
        With CREST all queries are defined in the CREST server
        """
        pass

    def _get_payload(self, hash: str) -> dict:
        """get payload from crest server using request library

        Args:
            hash (str): the query part of the url as required by the REST api

        Raises:
            RuntimeError: when connection or query failed

        Returns:
            dict: the json content
        """
        import requests

        url = f"{self.crestServer}/payloads/data"
        params = {
            "format": "BLOB", 
            "hash": hash
            }
        preq = requests.Request(method='GET', url=url, params=params).prepare()
        with requests.Session() as session:
            try:
                resp = session.send(preq)
            except requests.ConnectionError as exc:
                log.error(f"Could not connect to crest server {self.crestServer} ({exc})")
                raise RuntimeError(f"Could not connect to CREST server {self.crestServer}")
        
        if resp.status_code != 200:
            log.error(f"Error: HTTP GET request '{preq.url}' failed")
            raise RuntimeError(f"Query {hash} to crest failed with status code {resp.status_code}")
        
        config = json.loads(resp.content, object_pairs_hook = odict)
        self.confirmConfigType(config)
        return config

    @staticmethod
    def _getDBSchemaName(dbalias) -> str:
        dblookupFile = ConfigDBLoader.getResolvedFileName("dblookup.xml", "CORAL_DBLOOKUP_PATH")
        dbp = ET.parse(dblookupFile)
        for logSvc in dbp.iter("logicalservice"):
            if logSvc.attrib["name"] != dbalias:
                continue
            oracleServer = ""
            for serv in logSvc.iter("service"):
                if serv.attrib["name"].startswith("oracle://"):
                    oracleService = serv.attrib["name"]
                    oracleServer = oracleService.split('/')[3]
                    return oracleServer
            raise RuntimeError(f"DB {dbalias} has no oracle services listed in {dblookupFile}")
        raise RuntimeError(f"DB {dbalias} is not listed in {dblookupFile}")

    def load(self) -> dict:
        # see SCHEMA_MAP in https://gitlab.cern.ch/crest-db/crest/-/blob/master/src/main/java/hep/crest/server/repositories/triggerdb/TriggerDb.java
        self.schema = ConfigCrestLoader._getDBSchemaName(self.dbalias)
        url_schema = {
            "ATLAS_CONF_TRIGGER_RUN3": "CONF_DATA_RUN3",
            "ATLAS_CONF_TRIGGER_MC_RUN3": "CONF_MC_RUN3",
            "ATLAS_CONF_TRIGGER_REPR_RUN3": "CONF_REPR_RUN3", 
        }.get(self.schema)
        if url_schema is None:
            raise RuntimeError(f"Oracle server {self.schema} is not implemented in the crest server {self.crestServer}")
        hash = f"triggerdb://{url_schema}/{self.configType.crestkey}/{self.dbkey}"
        config = self._get_payload(hash=hash)
        return config

    # proposed filename when writing config to file
    def getWriteFilename(self):
        return "{basename}_{schema}_{dbkey}.json".format(basename = self.configType.basename, schema = self.schema, dbkey = self.dbkey)

class TriggerConfigAccess:
    """ 
    base class to hold the configuration (dict)
    and provides basic functions to access and print
    """
    def __init__(self, configType, mainkey, filename = None, jsonString = None, dbalias = None, dbkey = None, useCrest=False, crestServer=""):
        self._getLoader(configType = configType, filename = filename, jsonString = jsonString, dbalias = dbalias, dbkey = dbkey, 
                        useCrest=useCrest, crestServer=crestServer)
        self._mainkey = mainkey
        self._config = None

    def _getLoader(self, *, configType, filename = None, jsonString = None, dbalias = None, dbkey = None,
                   useCrest:bool = False, crestServer:str = ""):
        if filename:
            self.loader = ConfigFileLoader( configType, filename )
        elif dbalias and dbkey:
            if useCrest:
                self.loader = ConfigCrestLoader(configType=configType, dbalias=dbalias, dbkey=dbkey, crestServer=crestServer)
            else:
                self.loader = ConfigDBLoader( configType, dbalias, dbkey)
        elif jsonString:
            self.loader = ConfigDirectLoader( configType, jsonString )
        else:
            raise RuntimeError("Neither input file, nor JSON nor db alias and key provided")

    def load(self):
        self._config = self.loader.load()

    def __str__(self):
        return str(self._config)

    def __iter__(self):
        return iter(self[self._mainkey])

    def __getitem__(self, item):
        return self._config[item]

    def __len__(self):
        return len(self[self._mainkey])

    def config(self):
        """ returns the configuration """
        return self._config

    def prettyPrint(self):
        if self._config:
            print(json.dumps(self._config, indent = 4, separators=(',', ': ')))
    
    def name(self):
        return self["name"]

    def filetype(self):
        return self["filetype"]

    def printSummary(self):
        """ print summary info, should be overwritten by derived classes """
        log.info("Configuration name: {0}".format(self.name()))
        log.info("Configuration size: {0}".format(len(self)))

    def writeFile(self, filename = None):
        if filename is None:
            filename: str = self.loader.getWriteFilename()
        with open(filename, 'w') as fh:
            json.dump(self.config(), fh, indent = 4, separators=(',', ': '))
            log.info("Wrote file %s", filename)
