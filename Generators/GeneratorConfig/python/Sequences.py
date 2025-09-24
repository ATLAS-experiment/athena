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
    from AthenaConfiguration.ComponentFactory import CompFactory
    AthSequencer = CompFactory.AthSequencer
    if sequence is EvgenSequence.Main:
        return AthSequencer(EvgenSequence.Main.value, Sequential=True)
    if sequence is EvgenSequence.Generator:
        return AthSequencer(EvgenSequence.Generator.value)
    if sequence is EvgenSequence.Fix:
        return AthSequencer(EvgenSequence.Fix.value)
    if sequence is EvgenSequence.PreFilter:
        return AthSequencer(EvgenSequence.PreFilter.value)
    if sequence is EvgenSequence.Test:
        return AthSequencer(EvgenSequence.Test.value)
    if sequence is EvgenSequence.Filter:
        return AthSequencer(EvgenSequence.Filter.value)
    if sequence is EvgenSequence.Post:
        return AthSequencer(EvgenSequence.Post.value)
