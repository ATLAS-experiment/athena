"""Common enums for the generator configuration

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
from enum import Enum


class EvgenSequence(Enum):
    Generator = "EvgenGenSeq"
    Fix = "EvgenFixSeq"
    PreFilter = "EvgenPreFilterSeq"
    Test = "EvgenTestSeq"
    Filter = "EvgenFilterSeq"
    Post = "EvgenPostSeq"


def EvgenSequenceFactory(sequence):
    """Factory function to return the AthSequencer instance based on the enum value."""
    from AthenaConfiguration.ComponentFactory import CompFactory
    AthSequencer = CompFactory.AthSequencer
    if sequence is EvgenSequence.Generator:
        return AthSequencer(EvgenSequence.Generator.value, StopOverride=True)
    if sequence is EvgenSequence.Fix:
        return AthSequencer(EvgenSequence.Fix.value, StopOverride=True)
    if sequence is EvgenSequence.PreFilter:
        return AthSequencer(EvgenSequence.PreFilter.value, StopOverride=True)
    if sequence is EvgenSequence.Test:
        return AthSequencer(EvgenSequence.Test.value, StopOverride=True)
    if sequence is EvgenSequence.Filter:
        return AthSequencer(EvgenSequence.Filter.value, StopOverride=True)
    if sequence is EvgenSequence.Post:
        return AthSequencer(EvgenSequence.Post.value, StopOverride=True)
