"""Common enums for the generator configuration

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
from enum import Enum


class EvgenSequence(Enum):
    Main = "EvgenMainSeq"
    Generator = "EvgenGenSeq"
    Fix = "EvgenFixSeq"
    PreFilter = "EvgenPreFilterSeq"
    Test = "EvgenTestSeq"
    Filter = "EvgenFilterSeq"
    Post = "EvgenPostSeq"


def EvgenSequenceFactory(sequence):
    """Factory function to return the AthSequencer instance based on the enum value."""
    from AthenaCommon.CFElements import seqAND, parAND
    if sequence is EvgenSequence.Main:
        return seqAND(EvgenSequence.Main.value)
    if sequence is EvgenSequence.Generator:
        return seqAND(EvgenSequence.Generator.value)
    if sequence is EvgenSequence.Fix:
        return parAND(EvgenSequence.Fix.value)
    if sequence is EvgenSequence.PreFilter:
        return parAND(EvgenSequence.PreFilter.value)
    if sequence is EvgenSequence.Test:
        return parAND(EvgenSequence.Test.value)
    if sequence is EvgenSequence.Filter:
        return parAND(EvgenSequence.Filter.value)
    if sequence is EvgenSequence.Post:
        return parAND(EvgenSequence.Post.value)
