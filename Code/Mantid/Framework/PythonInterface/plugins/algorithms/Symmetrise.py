from mantid import logger, mtd
from mantid.api import PythonAlgorithm, AlgorithmFactory, WorkspaceProperty
from mantid.kernel import Direction
from mantid.simpleapi import CreateWorkspace, CopyLogs, CopySample, CopyInstrumentParameters, SaveNexusProcessed

import math
import os.path
import numpy as np


class Symmetrise(PythonAlgorithm):

    def category(self):
        return "Workflow\\MIDAS;PythonAlgorithms"

    def summary(self):
        return "Takes an asymmetric S(Q,w) and makes it symmetric"

    def PyInit(self):
        self.declareProperty(WorkspaceProperty("Sample", "", Direction.Input),
                             doc='Sample to run with')
        self.declareProperty('XCut', 0.0, doc='X cut off value')

        self.declareProperty('Verbose', defaultValue=False,
                             doc='Switch verbose output Off/On')
        self.declareProperty('Plot', defaultValue=False,
                             doc='Switch plotting Off/On')
        self.declareProperty('Save', defaultValue=False,
                             doc='Switch saving result to nxs file Off/On')

        self.declareProperty(WorkspaceProperty("OutputWorkspace", "",
                             Direction.Output), doc='Name to call the output workspace.')

    def PyExec(self):
        from IndirectCommon import CheckHistZero, StartTime, EndTime

        StartTime('Symmetrise')
        self._setup()
        num_spectra, num_pts = CheckHistZero(self._sample)
        sample_x = mtd[self._sample].readX(0)

        if math.fabs(self._x_cut) < 1e-5:
            raise ValueError('XCut point is Zero')

        # Find range of values to flip
        delta_x = sample_x[1] - sample_x[0]

        negative_diff = np.absolute(sample_x + self._x_cut)
        negative_index = np.where(negative_diff < delta_x)[0][-1]
        self._check_bounds(negative_index, num_pts, label='Negative')

        positive_diff = np.absolute(sample_x + sample_x[negative_index])
        positive_index = np.where(positive_diff < delta_x)[0][-1]
        self._check_bounds(positive_index, num_pts, label='Positive')

        new_data_size = 2 * num_pts - (positive_index + negative_index) + 1

        if self._verbose:
            logger.notice('No. points = %d' % num_pts)
            logger.notice('Negative : at i =%d; x = %f'
                          % (negative_index, sample_x[negative_index]))
            logger.notice('Positive : at i =%d; x = %f'
                          % (positive_index, sample_x[positive_index]))

        zeros = np.zeros(new_data_size * num_spectra)
        CreateWorkspace(OutputWorkspace=self._output_workspace,
                        DataX=zeros, DataY=zeros, DataE=zeros,
                        NSpec=num_spectra)

        CopyLogs(InputWorkspace=self._sample, OutputWorkspace=self._output_workspace)
        # CopyInstrumentParameters(InputWorkspace=self._sample, OutputWorkspace=self._output_workspace)
        # CopySample(InputWorkspace=self._sample, OutputWorkspace=self._output_workspace)

        # For each spectrum copy positive values to the negative
        for index in xrange(num_spectra):
            x_in = mtd[self._sample].readX(index)
            y_in = mtd[self._sample].readY(index)
            e_in = mtd[self._sample].readE(index)

            x_out = np.zeros(new_data_size)
            y_out = np.zeros(new_data_size)
            e_out = np.zeros(new_data_size)

            # Left hand side of cut
            x_out[:num_pts - positive_index] = -x_in[num_pts:positive_index:-1]
            y_out[:num_pts - positive_index] = y_in[num_pts:positive_index:-1]
            e_out[:num_pts - positive_index] = e_in[num_pts:positive_index:-1]

            # Right hand side of cut
            x_out[num_pts - positive_index:] = x_in[negative_index:]
            y_out[num_pts - positive_index:] = y_in[negative_index:]
            e_out[num_pts - positive_index:] = e_in[negative_index:]

            mtd[self._output_workspace].setX(index, x_out)
            mtd[self._output_workspace].setY(index, y_out)
            mtd[self._output_workspace].setE(index, e_out)

        if self._save:
            self._save_output()

        if self._plot:
            self._plot_output()

        self.setProperty("OutputWorkspace", self._output_workspace)
        EndTime('Symmetrise')

    def _setup(self):
        """
        Get the algorithm properties.
        """
        self._sample = self.getPropertyValue('Sample')
        self._x_cut = self.getProperty('XCut').value

        self._verbose = self.getProperty('Verbose').value
        self._plot = self.getProperty('Plot').value
        self._save = self.getProperty('Save').value

        self._output_workspace = self.getPropertyValue('OutputWorkspace')

    def _check_bounds(self, index, num_pts, label=''):
        """
        Check if the index falls within the bounds of the x range.
        Throws a ValueError if the x point falls outside of the range.

        @param index  - value of the index within the x range.
        @param num_pts - total number of points in the range.
        @param label - label to call the point if an error is thrown.
        """
        if index <= 0:
            raise ValueError('%s point %d < 0' % (label, index))
        elif index >= num_pts:
            raise ValueError('%s point %d > %d' % (label, index, num_pts))

    def _save_output(self):
        """
        Save the output workspace to the user's default working directory
        """
        from IndirectCommon import getDefaultWorkingDirectory
        workdir = getDefaultWorkingDirectory()
        file_path = os.path.join(workdir, self._output_workspace + '.nxs')
        SaveNexusProcessed(InputWorkspace=self._output_workspace,
                           Filename=file_path)

        if self._verbose:
            logger.notice('Output file : ' + file_path)

    def _plot_output(self):
        """
        Plot the first spectrum of the input and output workspace together.
        """
        from IndirectImport import import_mantidplot
        mp = import_mantidplot()
        mp.plotSpectrum([self._output_workspace, self._sample], 0)

# Register algorithm with Mantid
AlgorithmFactory.subscribe(Symmetrise)
