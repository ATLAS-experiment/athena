#!/bin/bash

export CLING_STANDARD_PCH=none #See bug ROOT-10789
python -m unittest -v DQDefects.tests
