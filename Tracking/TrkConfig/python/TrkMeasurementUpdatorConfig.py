# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkMeasurementUpdator_xk and TrkMeasurementUpdator packages
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TrkConfig.TrkConfigFlags import KalmanUpdatorType

# Relative timing results from ATLASRECTS-6755
# normalized to Trk::KalmanUpdator_xk time
# KalmanUpdator_xk   :   1
# KalmanUpdatorSMatrix : 1.45
# KalmanUpdator : 3.8


def KalmanUpdator_xkCfg(flags, name='KalmanUpdator_xk', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.Trk.KalmanUpdator_xk(name, **kwargs))
    return result


def KalmanUpdatorCfg(flags, name='KalmanUpdator', **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.Trk.KalmanUpdator(name, **kwargs))
    return acc


def InDetUpdatorCfg(flags, name='InDetUpdator', **kwargs):
    if flags.Detector.GeometryITk:
        name = name.replace("InDet", "ITk")

    acc = ComponentAccumulator()

    tool = None
    if flags.Tracking.kalmanUpdator == KalmanUpdatorType.KalmanUpdator_xk:
        tool = CompFactory.Trk.KalmanUpdator_xk(name, **kwargs)
    elif flags.Tracking.kalmanUpdator == KalmanUpdatorType.KalmanWeightUpdator:
        tool = CompFactory.Trk.KalmanWeightUpdator(name, **kwargs)
    elif flags.Tracking.kalmanUpdator == KalmanUpdatorType.KalmanUpdatorSMatrix:
        tool = CompFactory.Trk.KalmanUpdatorSMatrix(name, **kwargs)
    elif flags.Tracking.kalmanUpdator == KalmanUpdatorType.KalmanUpdator:
        tool = CompFactory.Trk.KalmanUpdator(name, **kwargs)

    acc.setPrivateTools(tool)
    return acc


def ITkUpdatorCfg(flags, name='ITkUpdator', **kwargs):
    return InDetUpdatorCfg(flags, name, **kwargs)
