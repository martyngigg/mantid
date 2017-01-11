import unittest
import mantid

from sans.common.file_information import (SANSFileInformationFactory, SANSFileInformation, FileType,
                                          SANSInstrument, get_instrument_paths_for_sans_file)
from mantid.kernel import DateAndTime


class SANSFileInformationTest(unittest.TestCase):
    def test_that_can_extract_information_from_file_for_SANS2D_single_period_and_ISISNexus(self):
        # Arrange
        # The file is a single period
        file_name = "SANS2D00022024"
        factory = SANSFileInformationFactory()

        # Act
        file_information = factory.create_sans_file_information(file_name)

        # Assert
        self.assertTrue(file_information.get_number_of_periods() == 1)
        self.assertTrue(file_information.get_date() == DateAndTime("2013-10-25T14:21:19"))
        self.assertTrue(file_information.get_instrument() == SANSInstrument.SANS2D)
        self.assertTrue(file_information.get_type() == FileType.ISISNexus)
        self.assertTrue(file_information.get_run_number() == 22024)
        self.assertFalse(file_information.is_event_mode())
        self.assertFalse(file_information.is_added_data())

    def test_that_can_extract_information_from_file_for_LOQ_single_period_and_raw_format(self):
        # Arrange
        # The file is a single period
        file_name = "LOQ48094"
        factory = SANSFileInformationFactory()

        # Act
        file_information = factory.create_sans_file_information(file_name)

        # Assert
        self.assertTrue(file_information.get_number_of_periods() == 1)
        self.assertTrue(file_information.get_date() == DateAndTime("2008-12-18T11:20:58"))
        self.assertTrue(file_information.get_instrument() == SANSInstrument.LOQ)
        self.assertTrue(file_information.get_type() == FileType.ISISRaw)
        self.assertTrue(file_information.get_run_number() == 48094)
        self.assertFalse(file_information.is_added_data())

    def test_that_can_extract_information_from_file_for_SANS2D_multi_period_event_and_nexus_format(self):
        # Arrange
        # The file is a multi period and event-based
        file_name = "LARMOR00003368"
        factory = SANSFileInformationFactory()

        # Act
        file_information = factory.create_sans_file_information(file_name)

        # Assert
        self.assertTrue(file_information.get_number_of_periods() == 4)
        self.assertTrue(file_information.get_date() == DateAndTime("2015-06-05T14:43:49"))
        self.assertTrue(file_information.get_instrument() == SANSInstrument.LARMOR)
        self.assertTrue(file_information.get_type() == FileType.ISISNexus)
        self.assertTrue(file_information.get_run_number() == 3368)
        self.assertTrue(file_information.is_event_mode())
        self.assertFalse(file_information.is_added_data())

    def test_that_can_extract_information_for_added_histogram_data_and_nexus_format(self):
        # Arrange
        # The file is a single period, histogram-based and added
        file_name = "SANS2D00022024-add"
        factory = SANSFileInformationFactory()

        # Act
        file_information = factory.create_sans_file_information(file_name)

        # Assert
        self.assertTrue(file_information.get_number_of_periods() == 1)
        self.assertTrue(file_information.get_date() == DateAndTime("2013-10-25T14:21:19"))
        self.assertTrue(file_information.get_instrument() == SANSInstrument.SANS2D)
        self.assertTrue(file_information.get_type() == FileType.ISISNexusAdded)
        self.assertTrue(file_information.get_run_number() == 22024)
        self.assertFalse(file_information.is_event_mode())
        self.assertTrue(file_information.is_added_data())

    def test_that_can_extract_information_for_LARMOR_added_event_data_and_multi_period_and_nexus_format(self):
        # Arrange
        # The file is a single period, histogram-based and added
        file_name = "LARMOR00013065-add"
        factory = SANSFileInformationFactory()

        # Act
        file_information = factory.create_sans_file_information(file_name)

        # Assert
        self.assertTrue(file_information.get_number_of_periods() == 4)
        self.assertTrue(file_information.get_date() == DateAndTime("2016-10-12T04:33:47"))
        self.assertTrue(file_information.get_instrument() == SANSInstrument.LARMOR)
        self.assertTrue(file_information.get_type() == FileType.ISISNexusAdded)
        self.assertTrue(file_information.get_run_number() == 13057)
        self.assertTrue(file_information.is_event_mode())
        self.assertTrue(file_information.is_added_data())


class SANSFileInformationGeneralFunctionsTest(unittest.TestCase):
    def test_that_finds_idf_and_ipf_paths(self):
        # Arrange
        file_name = "SANS2D00022024"
        # Act
        idf_path, ipf_path = get_instrument_paths_for_sans_file(file_name)
        # Assert
        self.assertTrue(idf_path is not None)
        self.assertTrue(ipf_path is not None)
        self.assertTrue("Definition" in idf_path)
        self.assertTrue("Parameters" in ipf_path)


if __name__ == '__main__':
    unittest.main()
