# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import GaudiConfig2.semantics
from GaudiKernel.DataHandle import DataHandle
import re


class MapMergeNoReplaceSemantics(GaudiConfig2.semantics.MappingSemantics):
    '''
    Extend the mapping-semantics with a merge-method that merges two mappings as long as they do not have different values for the same key
    Use 'mapMergeNoReplace<T>' as fifth parameter of the Gaudi::Property<T> constructor
    to invoke this merging method.
    '''
    __handled_types__ = (re.compile(r"^mapMergeNoReplace<.*>$"),)
    def __init__(self, cpp_type):
        super(MapMergeNoReplaceSemantics, self).__init__(cpp_type)

    def merge(self,a,b):
        for k in b.keys():
            if k in a and b[k] != a[k]:
                raise ValueError('conflicting values in map under key %r and %r %r' % (k, b[k], a[k]))
            a[k] = b[k]
        return a


class VarHandleKeySemantics(GaudiConfig2.semantics.PropertySemantics):
    '''
    Semantics for all data handle keys (Read, Write, Decor, Cond).
    '''
    __handled_types__ = (re.compile(r"SG::.*HandleKey<.*>$"),)

    def __init__(self, cpp_type):
        super().__init__(cpp_type)
        # Deduce actual handle type
        self._type = next(GaudiConfig2.semantics.extract_template_args(cpp_type))
        self._isCond = 'CondHandle' in cpp_type

        if cpp_type.startswith("SG::Read"):
            self._mode = "R"
        elif cpp_type.startswith("SG::Write"):
            self._mode = "W"
        else:
            raise TypeError(f"C++ type {cpp_type} not supported")

    def store(self, value):
        if isinstance(value, DataHandle):
            v = value.Path
        elif isinstance(value, str):
            v = value
        else:
            raise TypeError(f"cannot assign {value!r} ({type(value)}) to {self.name}"
                            ", expected string or DataHandle")
        return DataHandle(v, self._mode, self._type, self._isCond)


class VarHandleArraySematics(GaudiConfig2.semantics.SequenceSemantics):
    '''
    Treat VarHandleKeyArrays like arrays of strings
    '''
    __handled_types__ = ("SG::VarHandleKeyArray",)

    class _ItemSemantics(GaudiConfig2.semantics.StringSemantics):
        """Semantics for an item (DataHandle) in a VarHandleKeyArray converting to string"""

        def __init__(self):
            super().__init__("std::string")

        def store(self, value):
            if isinstance(value, DataHandle):
                return value.Path
            elif isinstance(value, str):
                return value
            else:
                raise TypeError(f"cannot assign {value!r} ({type(value)}) to {self.name}"
                                ", expected string or DataHandle")

    def __init__(self, cpp_type):
        super().__init__(cpp_type, valueSem = self._ItemSemantics())

    def merge(self,bb,aa):
        for b in bb:
            if b not in aa:
                aa.append(b)
        return aa


from AthenaServices.ItemListSemantics import OutputStreamItemListSemantics

GaudiConfig2.semantics.SEMANTICS.append(VarHandleKeySemantics)
GaudiConfig2.semantics.SEMANTICS.append(VarHandleArraySematics)
GaudiConfig2.semantics.SEMANTICS.append(MapMergeNoReplaceSemantics)
GaudiConfig2.semantics.SEMANTICS.append(OutputStreamItemListSemantics)
