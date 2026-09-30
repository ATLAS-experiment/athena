#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""
EventGUIDLookupLib.py

Provenance GUID resolution, adapted from the "Self reference and
Provenance" block of
  Database/EventIndex/EventIndexProducer/python/POOL2EI_Lib.py

- If the requested dataType IS the input file's own dataType, the GUID
  is in the *self reference* (the current file's own DataHeader entry).
- Otherwise, the GUID is in the *ancestor provenance* record for that
  stream, and only exists at all for the handful of streams POOL2EI_Lib
  recognizes there (RAW/ESD/AOD/RDO/HITS/EVGEN/Embedding) -- see
  _KNOWN_ANCESTOR_KEYS.
"""

import re

_TOKEN_DB_RE = re.compile(r'\[DB=([0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-'
                           r'[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12})\]')

# ATLAS data type -> internal DataHeader/provenance stream key
_STREAM_KEY = {
    'RAW': 'StreamRAW',
    'ESD': 'StreamESD',
    'AOD': 'StreamAOD',
    'RDO': 'StreamRDO',
    'HITS': 'StreamHITS',
    'EVNT': 'StreamEVGEN',
}

_KNOWN_ANCESTOR_KEYS = ('StreamRAW', 'StreamAOD', 'StreamESD', 'StreamRDO',
                         'StreamHITS', 'StreamEVGEN', 'EmbeddingStream')


def _streamKey(dataType):
    """Map --dataType/--inputDataType onto the DataHeader key. Types with
    no fixed internal name (DAOD_PHYS, DAOD_PHYSLITE) fall back to
    'Stream<dataType>' -- only usable as a self-reference, never as a
    recognized ancestor key (see _KNOWN_ANCESTOR_KEYS)."""
    return _STREAM_KEY.get(dataType, 'Stream' + dataType)


def guid2string(guid):
    return "{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}".format(
        guid.data1(), guid.data2(), guid.data3(),
        ord(guid.data4(0)[0]), ord(guid.data4(1)[0]),
        ord(guid.data4(2)[0]), ord(guid.data4(3)[0]),
        ord(guid.data4(4)[0]), ord(guid.data4(5)[0]),
        ord(guid.data4(6)[0]), ord(guid.data4(7)[0]))


def token2string(tk, replace_empty_cntID=False):
    cntID = tk.contID()
    if replace_empty_cntID and cntID == "":
        cntID = "POOLContainer(DataHeader)"
    return "[DB={}][CNT={}][CLID={}][TECH={:08X}][OID={:016X}-{:016X}]".format(
        guid2string(tk.dbID()), cntID, guid2string(tk.classID()),
        tk.technology(), tk.oid().first, tk.oid().second)


def _resolveAncestorProvenance(dh, requestedKey):
    if dh.sizeProvenance() == 0:
        return None
    guid = None
    prv = dh.beginProvenance()
    for _ in range(dh.sizeProvenance()):
        key = prv.getKey()
        if key.startswith("Output"):
            key = key[6:]
        if key.startswith("Input"):
            key = key[5:]
        if key in _KNOWN_ANCESTOR_KEYS and key == requestedKey and guid is None:
            tk = prv.getToken()
            stk = token2string(tk, replace_empty_cntID=(key != 'StreamRAW'))
            m = _TOKEN_DB_RE.search(stk)
            if m:
                guid = m.group(1)
            del tk
        del key
        prv += 1
    del prv
    return guid


def _resolveSelfReference(dh, requestedKey):
    if dh.size() == 0:
        return None
    guid = None
    dhe = dh.begin()
    for _ in range(dh.size()):
        key = dhe.getKey()
        if key == requestedKey and guid is None:
            tk = dhe.getToken()
            stk = token2string(tk, replace_empty_cntID=True)
            m = _TOKEN_DB_RE.search(stk)
            if m:
                guid = m.group(1)
            del tk
        del key
        dhe += 1
    dh.end()
    return guid


def resolveProvenanceGuid(store, dataType, inputDataType):
    """Return the provenance GUID string for `dataType`, given the input
    file is actually `inputDataType`, for the current event -- call from
    inside an algorithm's execute(). Returns None if not found."""

    dh = store.retrieve('DataHeader', 'EventSelector')
    requestedKey = _streamKey(dataType)

    if dataType == inputDataType:
        return _resolveSelfReference(dh, requestedKey)
    else:
        return _resolveAncestorProvenance(dh, requestedKey)
