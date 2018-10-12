# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#pylint: disable=invalid-name
from __future__ import (absolute_import, division, print_function)
from qtpy.QtWidgets import QSizePolicy
from MPLwidgets import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure


class MplFigureCanvas(FigureCanvas):
    """  A customized Qt widget for matplotlib figure.
    It can be used to replace GraphicsView of QtGui
    """

    def __init__(self, parent):
        """  Initialization
        """
        # Instantialize matplotlib Figure
        self.fig = Figure()
        self.axes = self.fig.add_subplot(111)

        # Initialize parent class and set parent
        FigureCanvas.__init__(self, self.fig)
        self.setParent(parent)

        # Set size policy to be able to expanding and resizable with frame
        self.setSizePolicy(QSizePolicy.Expanding,
                           QSizePolicy.Expanding)
        self.updateGeometry()

    def plot(self, x, y):
        """ Plot a set of data
        Argument:
        - x: numpy array X
        - y: numpy array Y
        """
        self.x = x
        self.y = y
        self.axes.plot(self.x, self.y)

    def getPlot(self):
        """ return figure's axes to expose the matplotlib figure to PyQt client
        """
        return self.axes
