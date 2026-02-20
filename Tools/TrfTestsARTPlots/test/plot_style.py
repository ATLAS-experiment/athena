# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""This module provides the ``set_atlas()`` convenience function."""

import logging
import matplotlib

logger = logging.getLogger("mATLASplotlib.style")


def set_atlas():
    """Set the plotting style to ATLAS-style and then point this function to 'None' so that it can only be called once. Called on canvas creation."""
    logger.info("Setting ATLAS style")

    # Set figure layout
    matplotlib.rcParams["figure.figsize"] = (24, 24)
    matplotlib.rcParams["figure.facecolor"] = "white"
    matplotlib.rcParams["figure.subplot.bottom"] = 0.16
    matplotlib.rcParams["figure.subplot.top"] = 0.95
    matplotlib.rcParams["figure.subplot.left"] = 0.16
    matplotlib.rcParams["figure.subplot.right"] = 0.95

    # Set font options
    matplotlib.rcParams["font.family"] = "sans-serif"
    matplotlib.rcParams["font.sans-serif"] = "Helvetica, helvetica, Nimbus Sans L, Mukti Narrow, FreeSans"  # alternatives if helvetica is unavailable
    matplotlib.rcParams["font.cursive"] = "Apple Chancery, Textile, Zapf Chancery, Sand, Script MT, Felipa, cursive, Helvetica, helvetica"
    matplotlib.rcParams["font.size"] = 32
    matplotlib.rcParams["mathtext.fontset"] = "custom"
    matplotlib.rcParams["mathtext.default"] = "sf"
    matplotlib.rcParams["mathtext.cal"] = "cursive"
    matplotlib.rcParams["mathtext.bf"] = "Helvetica:bold"
    matplotlib.rcParams["mathtext.it"] = "Helvetica:italic"
    matplotlib.rcParams["mathtext.rm"] = "serif"
    matplotlib.rcParams["mathtext.sf"] = "Helvetica"
    matplotlib.rcParams["mathtext.tt"] = "monospace"

    # Set axes options
    matplotlib.rcParams["axes.labelsize"] = 32
    matplotlib.rcParams["axes.titlesize"] = 32
    matplotlib.rcParams["xtick.bottom"] = True
    matplotlib.rcParams["xtick.top"] = True
    matplotlib.rcParams["xtick.direction"] = "in"
    matplotlib.rcParams["xtick.labelsize"] = 32
    matplotlib.rcParams["xtick.major.size"] = 12
    matplotlib.rcParams["xtick.minor.size"] = 6
    matplotlib.rcParams["ytick.left"] = True
    matplotlib.rcParams["ytick.right"] = True
    matplotlib.rcParams["ytick.direction"] = "in"
    matplotlib.rcParams["ytick.labelsize"] = 32
    matplotlib.rcParams["ytick.major.size"] = 14
    matplotlib.rcParams["ytick.minor.size"] = 7

    # Set line options
    matplotlib.rcParams["lines.markersize"] = 8
    matplotlib.rcParams["lines.linewidth"] = 1

    # Set legend options
    matplotlib.rcParams["legend.numpoints"] = 1
    matplotlib.rcParams["legend.fontsize"] = 32
    matplotlib.rcParams["legend.labelspacing"] = 0.3
    matplotlib.rcParams["legend.frameon"] = False

    # Disable calling this function again
    #set_atlas.func_code = (lambda: None).func_code

    matplotlib.rcParams["figure.titlesize"] = 32

