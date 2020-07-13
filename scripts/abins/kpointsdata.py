# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +
import numpy as np

from abins.constants import (ACOUSTIC_PHONON_THRESHOLD, ALL_KEYWORDS_K_DATA,
                             COMPLEX_ID, FLOAT_ID, GAMMA_POINT, SMALL_K)


class KpointsData:
    """Class storing atomic frequencies and displacements at specific k-points

    The data should be provided as a dictionary with the following form:

    data = {"frequencies": numpy.array,
            "atomic_displacements: numpy.array,
            "weights": numpy.array,
            "k_vectors": numpy.array}


    Each entry in the dictionary corresponds to all k-points. Each item
    in the dictionary is a numpy array. The meaning of keys in the
    dictionary is as follows:

    "weights" - weights of all k-points; weights.shape == (num_k,);

    "k_vectors"  - k_vectors of all k-points;  k_vectors.shape == (num_k, 3)

    "frequencies" - frequencies for all k-points; frequencies.shape == (num_k, num_freq)

    "atomic_displacements - atomic displacements for all k-points;
                            atomic_displacements.shape == (num_k, num_atoms, num_freq, 3)

    """

    def __init__(self, *, items):
        """
        :param num_k: total number of k-points (int)
        :param num_atoms: total number of atoms in the unit cell (int)
        :param items: dict of numpy arrays "weights", "k_vectors", "frequencies" and "atomic_displacements"
        """
        super(KpointsData, self).__init__()

        self._data = {}

        if not isinstance(items, dict):
            raise TypeError("'items' for KpointsData should be a dictionary.")

        if not sorted(items.keys()) == sorted(ALL_KEYWORDS_K_DATA):
            raise ValueError("Invalid structure of the items dictionary.")

        dim = 3

        # unit_cell
        unit_cell = items["unit_cell"]
        if not (isinstance(unit_cell, np.ndarray)
                and unit_cell.shape == (dim, dim)
                and unit_cell.dtype.num == FLOAT_ID):

            raise ValueError("Invalid values of unit cell vectors.")

        #  "weights"
        weights = items["weights"]
        num_k = weights.size

        if not (isinstance(weights, np.ndarray)
                and weights.dtype.num == FLOAT_ID
                and np.allclose(weights, weights[weights >= 0])):

            raise ValueError("Invalid value of weights.")

        #  "k_vectors"
        k_vectors = items["k_vectors"]

        if not (isinstance(k_vectors, np.ndarray)
                and k_vectors.shape == (num_k, dim)
                and k_vectors.dtype.num == FLOAT_ID):

            raise ValueError("Invalid value of k_vectors.")

        #  "frequencies"
        frequencies = items["frequencies"]
        num_freq = frequencies.shape[1]
        if not (isinstance(frequencies, np.ndarray)
                and frequencies.shape == (num_k, num_freq)
                and frequencies.dtype.num == FLOAT_ID):
            raise ValueError("Invalid value of frequencies.")

        # "atomic_displacements"
        atomic_displacements = items["atomic_displacements"]
        num_atoms = atomic_displacements.shape[1]

        if not (isinstance(
                atomic_displacements, np.ndarray)
                and atomic_displacements.shape == (weights.size, num_atoms, num_freq, dim)
                and atomic_displacements.dtype.num == COMPLEX_ID):

            raise ValueError("Invalid value of atomic_displacements.")

        # remove data which correspond to imaginary frequencies
        freq_dic = {}
        atomic_displacements_dic = {}
        weights_dic = {}
        k_vectors_dic = {}

        indx = frequencies > ACOUSTIC_PHONON_THRESHOLD
        for k in range(frequencies.shape[0]):
            freq_dic[str(k)] = frequencies[k, indx[k]]
            for atom in range(num_atoms):
                temp = atomic_displacements[k]
                atomic_displacements_dic[str(k)] = temp[:, indx[k]]

            weights_dic[str(k)] = weights[k]
            k_vectors_dic[str(k)] = k_vectors[k]

        self._data["unit_cell"] = unit_cell
        self._data["frequencies"] = freq_dic
        self._data["atomic_displacements"] = atomic_displacements_dic
        self._data["k_vectors"] = k_vectors_dic
        self._data["weights"] = weights_dic

    def get_gamma_point_data(self):
        """
        Extracts k points data only for Gamma point.
        :returns: dictionary with data only for Gamma point
        """
        gamma_pkt_index = -1

        # look for index of Gamma point
        for k in self._data["k_vectors"]:
            if np.linalg.norm(self._data["k_vectors"][k]) < SMALL_K:
                gamma_pkt_index = k
                break
        if gamma_pkt_index == -1:
            raise ValueError("Gamma point not found.")

        k_points = {"weights": {GAMMA_POINT: self._data["weights"][gamma_pkt_index]},
                    "k_vectors": {GAMMA_POINT: self._data["k_vectors"][gamma_pkt_index]},
                    "frequencies": {GAMMA_POINT: self._data["frequencies"][gamma_pkt_index]},
                    "atomic_displacements": {GAMMA_POINT: self._data["atomic_displacements"][gamma_pkt_index]}
                    }

        return k_points

    def extract(self):

        return self._data

    def __str__(self):
        return "K-points data"
