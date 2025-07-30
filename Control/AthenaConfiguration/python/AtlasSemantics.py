# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import GaudiConfig2.semantics
from GaudiKernel.DataHandle import DataHandle
import re


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
GaudiConfig2.semantics.SEMANTICS.append(OutputStreamItemListSemantics)
