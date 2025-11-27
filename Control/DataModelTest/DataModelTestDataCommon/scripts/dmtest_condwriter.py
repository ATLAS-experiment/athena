#!/usr/bin/env python
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Script to create an AthenaAttributeList with a single attribute "xint".
# Usage example: dmtest_condwriter.py --rs=1 --ls=1 'sqlite://;schema=test.db;dbname=OFLP200' AttrList_noTag 42
# CREST file system: dmtest_condwriter.py --rs=1 --ls=1 --tag=Test_AttrList_noTag --xint=42 --gtag=TEST-HLT-CREST --label='/DMTest/TestAttrList' --host='/tmp/crest_dump'

import sys,os
import argparse
import json
import tempfile
os.environ['CLING_STANDARD_PCH'] = 'none' #See bug ROOT-10789
from PyCool import cool
from CoolConvUtilities import AtlCoolLib, AtlCoolTool
from pycrest.api.crest_api import CrestApi
from pycrest.api.crest_fs_api import CrestApiFs
from hep.crest.client.models import (
    IovSetDto, HTTPResponse, TagMetaSetDto, TagMetaDto, TagSetDto, TagDto, GlobalTagDto,
    GlobalTagSetDto, GlobalTagMapDto, GlobalTagMapSetDto, StoreSetDto, StoreDto, RunLumiInfoDto, RunLumiSetDto)
from hep.crest.client import ApiException

import logging
log = logging.getLogger('dmtest_condwriter')

class createTestDB(AtlCoolLib.coolTool):

    def setup(self,args):
        # set values of non-optional parameters
        self.tag=str(args[0])
        self.xint=int(args[1])
        self.folder=args[2] if len(args)>2 else '/DMTest/TestAttrList'

    def usage(self):
        """ Define the additional syntax for options """
        self._usage1()
        print ('TAG xint [Folder]')
        self._usage2()
        
    def execute(self):

        # do update - setup folder specification and create if needed
        spec = cool.RecordSpecification()
        spec.extend("xint", cool.StorageType.Int32)
        print (">== Store object in folder", self.folder)
        cfolder = AtlCoolLib.ensureFolder(self.db, self.folder, spec,
                                          AtlCoolLib.athenaDesc(self.runLumi, 'AthenaAttributeList'),
                                          cool.FolderVersioning.MULTI_VERSION)
        if (cfolder is None): sys.exit(1)
        # now write data
        payload = cool.Record(spec)
        payload['xint'] = self.xint
        print ('>== Store object with IOV [',self.since,',',self.until,'] and tag',self.tag,'xint',self.xint)
        try:
            if (self.tag=="HEAD"):
                cfolder.storeObject(self.since,self.until,payload,0)
            else:
                cfolder.storeObject(self.since,self.until,payload,0,self.tag)
            print (">== Storing COOL object succeeded. Current content:")
        except Exception:
            import traceback
            traceback.print_exc()
            print ('>== Storing COOL object FAILED')
            sys.exit(1)

        # print full content
        act = AtlCoolTool.AtlCoolTool(self.db)
        print (act.more(self.folder))


def store_in_crest(parser):
    """
    Store xint into CREST
    """
    args, unknown = parser.parse_known_args()

    def require_arg(arg_val, arg_name):
        if arg_val is None:
            parser.print_help()
            sys.exit(1)
        return arg_val

    if not args.remove:
        runNumber = require_arg(args.r, 'r')
        lumiSince = require_arg(args.ls, 'ls')
        xint = require_arg(args.xint, 'xint')
        
    # Required in all cases
    host = require_arg(args.host, 'host')
    tag = require_arg(args.tag, 'tag')
    gtag = require_arg(args.gtag, 'gtag')
    label = require_arg(args.label, 'label')

    # Optional with default
    record = args.record if args.record is not None else 'test_record'

    if args.remove and host.startswith(tempfile.gettempdir()):
        print("Only remove tag from CREST server")
        sys.exit(1)

    # Instantiate the CREST API
    if host.startswith(tempfile.gettempdir()):
        api_instance = CrestApiFs(host)
        log.debug(f'>== Using CREST file system')
    else:
        log.debug(f'>== Using CREST server: {host}')
        api_instance = CrestApi(host=host)

    # Create or find the global tag
    gtag_dto=GlobalTagDto(
        name=gtag,
        description='Test global tag for hlt workflow',
        release='2.0',
        scenario='test',
        workflow='T',
        validity=0,
        type='t'
    )

    log.debug(f'>== Looking for global tag {gtag} in CREST ...')
    try:
        existing_gtag = None
        if host.startswith('http'):
            existing_gtag = api_instance.find_global_tag(name=gtag)
        elif os.path.exists(f'{host}/globaltags/{gtag}/globaltag.json'):
            existing_gtag = api_instance.find_global_tag(name=gtag)
        if existing_gtag is not None and isinstance(existing_gtag, GlobalTagDto):
            log.debug(f'>== Found existing global tag {existing_gtag.name}. Will not recreate.')
        else:
            log.debug(f'>== Creating new global tag: {gtag}')
            if host.startswith('http'):
                api_response = api_instance.create_global_tag(gtag_dto,'false')
            else:
                api_response = api_instance.create_global_tag(gtag_dto)
            if isinstance(api_response, GlobalTagDto):
                log.debug(f'>== Created global tag: {api_response.name}')
            else:
                print(f'>== Error creating global tag {gtag}: {api_response}')
                sys.exit(1)
    except Exception as e:
        print('Exception when calling CrestApi->create_global_tag:')
        print(e)
        sys.exit(1)

    # If remove is specified, remove the tag
    if args.remove:
        log.debug(f'>== Removing global tag map for tag: {tag}')
        try:
            api_instance.delete_global_tag_map(gtag, tag, label)
        except Exception as e:
            print('Exception when calling CrestApi->delete_global_tag_map:')
            print(e)
            sys.exit(1)
        log.debug(f'>== Removing tag with name: {tag}')
        try:
            api_instance.remove_tag(tag)
        except Exception as e:
            print('Exception when calling CrestApi->remove_tag:')
            print(e)
            sys.exit(1)
        return

    # Tag description
    description = '<timeStamp>run-lumi</timeStamp><addrHeader><address_header service_type=\"71\" clid=\"40774348\" /></addrHeader><typeName>AthenaAttributeList</typeName>'

    tag_info = {
        'channel_list': [{'0': 'channel_0'}],
        'node_description': description,
        'payload_spec': [{'xint':'Int32'}]
    }

    # Create or find the tag
    tag_dto = TagDto(
        name=tag,
        description=description,
        time_type='run-lumi',
        payload_spec='xint:Int32',
        status='UNLOCKED',
        synchronization='ALL',
        last_validated_time=-1,
        end_of_validity=-1
    )

    tag_meta_dto = TagMetaDto(
        tag_name=tag,
        description=description,
        tag_info=json.dumps(tag_info),
        chansize=1,
        colsize=1
    )

    log.debug(f'>== Looking for tag {tag} in CREST ...')
    try:
        existing_tag = None
        if host.startswith('http'):
            existing_tag = api_instance.find_tag(name=tag)
        elif os.path.exists(f'{host}/tags/{tag}/tag.json'):
            existing_tag = api_instance.find_tag(name=tag)
        if existing_tag and isinstance(existing_tag, TagDto):
            log.debug(f'>== Found existing tag {existing_tag.name}. Will not recreate.')
        else:
            log.debug(f'>== Creating new tag: {tag}')
            api_response = api_instance.create_tag(tag_dto)
            if isinstance(api_response, TagDto):
                log.debug(f'>== Created new tag with name: {api_response.name}')
            else:
                print(f'>== Error creating tag {tag}: {api_response}')
                sys.exit(1)
    except Exception as e:
        print('Exception when calling CrestApi->create_tag:')
        print(e)
        sys.exit(1)

    # Create or find tag metadata
    log.debug(f'>== Looking for tag metadata for tag {tag} ...')
    try:
        tag_meta = None
        if host.startswith('http'):
            tag_meta = api_instance.find_tag_meta(name=tag)
        elif os.path.exists(f'{host}/tags/{tag}/tagmetainfo.json'):
            tag_meta = api_instance.find_tag_meta(name=tag)
        if tag_meta and isinstance(tag_meta, TagMetaSetDto) and len(tag_meta.resources) > 0:
            log.debug(f'>== Found existing metadata for tag {tag_meta.resources[0].tag_name}. Will not recreate.')
        elif tag_meta and isinstance(tag_meta, TagMetaDto):
            log.debug(f'>== Found existing metadata for tag {tag_meta.tag_name}. Will not recreate.')
        else:
            log.debug(f'>== Creating new tag metadata for tag: {tag}')
            api_response = api_instance.create_tag_meta(tag_meta_dto)
            if isinstance(api_response, TagMetaDto):
                log.debug(f'>== Created tag meta for tag: {api_response.tag_name}')
            else:
                print(f'>== Error creating tag meta for tag {tag}: {api_response}')
                sys.exit(1)
    except Exception as e:
        print('Exception when calling CrestApi->create_tag_meta:')
        print(e)
        sys.exit(1)

    # Prepare the actual payload
    # If runNumber and lumiSince are set, we store at that run-lumi
    # We'll store the same integer in 'xint'
    sinfo = {'run-lumi': f'{runNumber}-{lumiSince}'}
    lumi_pyld = {'0': [int(xint)]}

    # We will store at the 'since' = lumiSince
    sdto = StoreDto(
        since=int(lumiSince),
        data=json.dumps(lumi_pyld),
        streamer_info=json.dumps(sinfo)
    )
    ssdto = StoreSetDto(
        size=1,
        format="StoreSetDto",
        datatype="data",
        resources=[sdto]
    )

    log.debug(f'>== Storing data in CREST: tag={tag}, run={runNumber}, LB={lumiSince}, xint={xint}')
    try:
        api_response = api_instance.store_data(tag=tag, store_set=ssdto)
    except Exception as e:
        print('Exception when calling CrestApi->store_data:')
        print(e)
        sys.exit(1)

    # Finally, create or find the global tag map
    global_tag_map_dto = GlobalTagMapDto(
        global_tag_name=gtag,
        record=record,
        label=label,
        tag_name=tag
    )
    log.debug(f'>== Checking/Creating global tag map for tag={tag}, gtag={gtag}, label={label}')
    try:
        globalTagMap = None
        if host.startswith('http'):
            globalTagMap = api_instance.find_global_tag_map(name=tag, mode='BackTrace')
        elif os.path.exists(f'{host}/globaltags/{gtag}/maps.json'):
            globalTagMap = api_instance.find_global_tag_map(name=gtag)
        if isinstance(globalTagMap, GlobalTagMapSetDto) and len(globalTagMap.resources) > 0:
            log.debug(f'>== Found existing global tag map entries for tag {tag}. Will not recreate.')
        else:
            log.debug(f'>== Creating global tag map for tag: {tag}')
            api_instance.create_global_tag_map(global_tag_map_dto)
    except Exception as e:
        print('Exception when calling CrestApi->create_global_tag_map:')
        print(e)
        sys.exit(1)


def main():

    parser = argparse.ArgumentParser(
        description='CREST Commands',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=(
            "Required to create a tag:\n"
            "--host=<host url or path> --tag=<tag> --gtag=<global tag> --label=<label> --r=<run number> --ls=<lumi since> --xint=<payload>\n\n"
            "Required to remove a tag:\n"
            "--remove --host=<host url> --tag=<tag> --gtag=<global tag> --label=<label>"
        )
    )

    # CREST-related arguments
    parser.add_argument('--host', default=None, help='Host URL of the CREST service')
    parser.add_argument('--tag', default=None, help='CREST tag name')
    parser.add_argument('--gtag', default=None, help='CREST global tag name')
    parser.add_argument('--label', default=None, help='Label used in the global tag map')
    parser.add_argument('--record', default=None, help='Record name for the global tag map')
    parser.add_argument('--remove', action='store_true', help='Remove the specified CREST tag')
    parser.add_argument('--r', type=int, default=int(0), help='Single run number (for CREST run-lumi info)')
    parser.add_argument('--ls', type=int, default=None, help='Single lumi block (for CREST run-lumi info)')
    parser.add_argument('--xint', type=int, default=None,
                        help='Integer payload value to store in COOL and/or CREST')
    parser.add_argument('--debug', action='store_true', help='Enable debugging information')

    args, unknown = parser.parse_known_args()

    logging_level = logging.ERROR
    if args.debug:
        logging_level = logging.DEBUG
    logging.basicConfig(level=logging_level, format = "%(name)s %(levelname)s: %(message)s")

    # If host is not found, do COOL storage
    if args.host is None:
        mytool = createTestDB('dmtest_condwriter.py',False,3,4,[])
    else:
        store_in_crest(parser)

if __name__ == "__main__":
    main()

