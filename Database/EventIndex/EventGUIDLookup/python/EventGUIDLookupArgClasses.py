# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Custom transform argument classes for EventGUIDLookup."""

import os

from PyJobTransforms.trfLogger import msg
import PyJobTransforms.trfArgClasses as trfArgClasses


class argEventGUIDFile(trfArgClasses.argFile):
    """Plain text output; nentries = number of non-blank lines, per file."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._metadataKeys['nentries'] = self._countLines
        self._metadataKeys['_exists'] = self._existsQuiet

    def _existsQuiet(self, files):
        present = []
        for fname in files:
            if self._urlType == 'posix' and '[' not in fname and not os.path.exists(fname):
                msg.warning('Optional output file %s was not produced', fname)
                self._fileMetadata.setdefault(fname, {})['_exists'] = False
            else:
                present.append(fname)
        if present:
            trfArgClasses.argFile._exists(self, present)

    def _countLines(self, files):
        for fname in files:
            if not os.path.exists(fname):
                continue
            with open(fname) as f:
                n = sum(1 for line in f if line.strip())
            self._fileMetadata.setdefault(fname, {})['nentries'] = n

    def getnentries(self, fast=False):
        total, found = 0, False
        for fname in self._value:
            if os.path.exists(fname):
                found = True
                with open(fname) as f:
                    total += sum(1 for line in f if line.strip())
        return total if found else None
