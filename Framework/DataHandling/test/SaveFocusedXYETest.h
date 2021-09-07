// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidDataHandling/SaveFocusedXYE.h"

#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/Axis.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidDataHandling/SaveGSS.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidFrameworkTestHelpers/WorkspaceCreationHelper.h"
#include "MantidKernel/Unit.h"
#include "MantidKernel/UnitFactory.h"

#include <Poco/File.h>
#include <cxxtest/TestSuite.h>

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

namespace {
/// This is the number of header lines in Fullprof format 10
static constexpr size_t MAX_HEADER_LENGTH = 6;
} // namespace

class SaveFocusedXYETest : public CxxTest::TestSuite {
public:
  static SaveFocusedXYETest *createSuite() { return new SaveFocusedXYETest(); }
  static void destroySuite(SaveFocusedXYETest *suite) { delete suite; }

  SaveFocusedXYETest() : m_tol(1e-08) {}

  void testHeaderTemperature() {
    using namespace Mantid::API;
    using namespace Mantid::DataObjects;
    using namespace Mantid::Kernel;
    Workspace2D_sptr workspace = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 1.0);
    workspace->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");
    std::shared_ptr<Units::Label> label = std::make_shared<Units::Label>();
    label->setLabel(std::string("Temperature"));
    workspace->getAxis(1)->unit() = label;

    Mantid::DataHandling::SaveFocusedXYE saveXYE;
    TS_ASSERT_THROWS_NOTHING(saveXYE.initialize());
    TS_ASSERT_EQUALS(saveXYE.isInitialized(), true);

    saveXYE.setProperty("InputWorkspace", workspace);
    std::string filename("focussedT.test");
    saveXYE.setPropertyValue("Filename", filename);
    filename = saveXYE.getPropertyValue("Filename"); // absolute path
    saveXYE.setProperty("SplitFiles", false);

    TS_ASSERT_THROWS_NOTHING(saveXYE.execute());

    Poco::File focusfile(filename);
    TS_ASSERT_EQUALS(focusfile.exists(), true);

    std::ifstream filestrm(filename.c_str());
    std::string line;
    const std::vector<std::string> expectedHeader{"XYDATA",
                                                  "# File generated by Mantid, Instrument ",
                                                  "# The X-axis unit is: Time-of-flight, The Y-axis unit is: ",
                                                  "# Data for spectra :0",
                                                  "TEMP 1 ",
                                                  "# Time-of-flight              Y                 E"};
    size_t lineNumber = 1;
    while (lineNumber <= expectedHeader.size()) {
      getline(filestrm, line);
      TS_ASSERT_EQUALS(line, expectedHeader[lineNumber - 1]);
      lineNumber++;
    }
  }

  void testHistogram() {
    using namespace Mantid::API;
    using namespace Mantid::DataObjects;
    Workspace2D_sptr workspace = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 1.0);
    workspace->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");

    TS_ASSERT_DIFFERS(workspace, std::shared_ptr<Workspace2D>());
    std::string resultWS("result");
    AnalysisDataService::Instance().add(resultWS, workspace);

    Mantid::DataHandling::SaveFocusedXYE saveXYE;
    TS_ASSERT_THROWS_NOTHING(saveXYE.initialize());
    TS_ASSERT_EQUALS(saveXYE.isInitialized(), true);

    saveXYE.setPropertyValue("InputWorkspace", resultWS);
    std::string filename("focussed.test");
    saveXYE.setPropertyValue("Filename", filename);
    filename = saveXYE.getPropertyValue("Filename"); // absolute path
    saveXYE.setProperty("SplitFiles", false);

    TS_ASSERT_THROWS_NOTHING(saveXYE.execute());

    Poco::File focusfile(filename);
    TS_ASSERT_EQUALS(focusfile.exists(), true);

    std::ifstream filestrm(filename.c_str());
    std::string line;
    const std::vector<std::string> expectedHeader{"XYDATA",
                                                  "# File generated by Mantid, Instrument ",
                                                  "# The X-axis unit is: Time-of-flight, The Y-axis unit is: ",
                                                  "# Data for spectra :0",
                                                  "# Spectrum 1 ",
                                                  "# Time-of-flight              Y                 E"};
    int bin_no(1);
    size_t lineNumber = 0;
    while (getline(filestrm, line)) {
      lineNumber++;
      std::istringstream is(line);
      if (lineNumber <= expectedHeader.size()) {
        TS_ASSERT_EQUALS(is.str(), expectedHeader[lineNumber - 1]);
        continue;
      }
      double x(0.0), y(0.0), e(0.);
      is >> x >> y >> e;
      switch (bin_no) {
      case 1:
        TS_ASSERT_DELTA(x, 1.5, m_tol);
        TS_ASSERT_DELTA(y, 2.0, m_tol);
        TS_ASSERT_DELTA(e, M_SQRT2, m_tol);
        break;
      case 2:
        TS_ASSERT_DELTA(x, 2.5, m_tol);
        TS_ASSERT_DELTA(y, 2.0, m_tol);
        TS_ASSERT_DELTA(e, M_SQRT2, m_tol);
        break;
      case 3:
        TS_ASSERT_DELTA(x, 3.5, m_tol);
        TS_ASSERT_DELTA(y, 2.0, m_tol);
        TS_ASSERT_DELTA(e, M_SQRT2, m_tol);
        break;
      default:
        TS_ASSERT(false);
      }
      ++bin_no;
    }
    filestrm.close();
    focusfile.remove();
    AnalysisDataService::Instance().remove(resultWS);
  }

  void testHistogramTOPAS() {
    using namespace Mantid::API;
    using namespace Mantid::DataObjects;
    Workspace2D_sptr workspace = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 1.0);
    workspace->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");

    TS_ASSERT_DIFFERS(workspace, std::shared_ptr<Workspace2D>());
    std::string resultWS("result");
    AnalysisDataService::Instance().add(resultWS, workspace);

    Mantid::DataHandling::SaveFocusedXYE saveXYE;
    TS_ASSERT_THROWS_NOTHING(saveXYE.initialize());
    TS_ASSERT_EQUALS(saveXYE.isInitialized(), true);

    saveXYE.setPropertyValue("InputWorkspace", resultWS);
    std::string filename("focussed.test");
    saveXYE.setPropertyValue("Filename", filename);
    filename = saveXYE.getPropertyValue("Filename"); // absolute path
    saveXYE.setProperty("SplitFiles", false);
    saveXYE.setProperty("Format", "TOPAS");

    TS_ASSERT_THROWS_NOTHING(saveXYE.execute());

    Poco::File focusfile(filename);
    TS_ASSERT_EQUALS(focusfile.exists(), true);

    std::ifstream filestrm(filename.c_str());
    std::string line;
    const std::vector<std::string> expectedHeader{
        "' File generated by Mantid, Instrument ",
        "' The X-axis unit is: Time-of-flight, The Y-axis unit is: ", "' Data for spectra :0", "' Spectrum 1 ",
        "' Time-of-flight              Y                 E"};
    int bin_no(1);
    size_t lineNumber = 0;
    while (getline(filestrm, line)) {
      lineNumber++;
      std::istringstream is(line);
      if (lineNumber <= expectedHeader.size()) {
        TS_ASSERT_EQUALS(is.str(), expectedHeader[lineNumber - 1]);
        continue;
      }
      double x(0.0), y(0.0), e(0.);
      is >> x >> y >> e;
      switch (bin_no) {
      case 1:
        TS_ASSERT_DELTA(x, 1.5, m_tol);
        TS_ASSERT_DELTA(y, 2.0, m_tol);
        TS_ASSERT_DELTA(e, M_SQRT2, m_tol);
        break;
      case 2:
        TS_ASSERT_DELTA(x, 2.5, m_tol);
        TS_ASSERT_DELTA(y, 2.0, m_tol);
        TS_ASSERT_DELTA(e, M_SQRT2, m_tol);
        break;
      case 3:
        TS_ASSERT_DELTA(x, 3.5, m_tol);
        TS_ASSERT_DELTA(y, 2.0, m_tol);
        TS_ASSERT_DELTA(e, M_SQRT2, m_tol);
        break;
      default:
        TS_ASSERT(false);
      }
      ++bin_no;
    }
    filestrm.close();
    focusfile.remove();
    AnalysisDataService::Instance().remove(resultWS);
  }

  void testSaveFocusedXYEWorkspaceGroups() {
    using namespace Mantid::API;
    using namespace Mantid::DataObjects;
    Workspace2D_sptr workspace = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 1.0);
    workspace->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");

    Workspace2D_sptr work_in1 = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 1.0);
    work_in1->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");

    Workspace2D_sptr work_in2 = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 1.0);
    work_in2->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");

    Workspace2D_sptr work_in3 = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 1.0);
    work_in3->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");

    Workspace2D_sptr work_in4 = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 1.0);
    work_in4->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");

    WorkspaceGroup_sptr wsSptr = WorkspaceGroup_sptr(new WorkspaceGroup);
    if (wsSptr) {
      AnalysisDataService::Instance().add("test_in", wsSptr);
      AnalysisDataService::Instance().add("test_in_1", work_in1);
      wsSptr->add("test_in_1");
      AnalysisDataService::Instance().add("test_in_2", work_in2);
      wsSptr->add("test_in_2");
      AnalysisDataService::Instance().add("test_in_3", work_in3);
      wsSptr->add("test_in_3");
      AnalysisDataService::Instance().add("test_in_4", work_in4);
      wsSptr->add("test_in_4");
    }
    Mantid::DataHandling::SaveFocusedXYE saveXYE;
    TS_ASSERT_THROWS_NOTHING(saveXYE.initialize());
    TS_ASSERT_EQUALS(saveXYE.isInitialized(), true);

    saveXYE.setPropertyValue("InputWorkspace", "test_in");
    std::string filename("focussed.txt");
    saveXYE.setPropertyValue("Filename", filename);
    filename = saveXYE.getPropertyValue("Filename"); // get the absolute path
    saveXYE.setProperty("SplitFiles", false);
    saveXYE.setPropertyValue("Append", "0");

    TS_ASSERT_THROWS_NOTHING(saveXYE.execute());
    Poco::File focusfile(filename);
    TS_ASSERT_EQUALS(focusfile.exists(), true);

    std::ifstream filestrm(filename.c_str());
    std::string line;
    int bin_no(1);
    size_t lineNumber = 0;
    while (getline(filestrm, line)) {
      lineNumber++;
      if (lineNumber <= MAX_HEADER_LENGTH || line[0] == '#')
        continue;
      double x(0.0), y(0.0), e(0.);
      std::istringstream is(line);
      is >> x >> y >> e;
      switch (bin_no) {
      case 1:
        TS_ASSERT_DELTA(x, 1.5, m_tol);
        TS_ASSERT_DELTA(y, 2.0, m_tol);
        TS_ASSERT_DELTA(e, M_SQRT2, m_tol);
        break;
      case 2:
        TS_ASSERT_DELTA(x, 2.5, m_tol);
        TS_ASSERT_DELTA(y, 2.0, m_tol);
        TS_ASSERT_DELTA(e, M_SQRT2, m_tol);
        break;
      case 3:
        TS_ASSERT_DELTA(x, 3.5, m_tol);
        TS_ASSERT_DELTA(y, 2.0, m_tol);
        TS_ASSERT_DELTA(e, M_SQRT2, m_tol);
        break;
      default:
        TS_ASSERT(false);
      }
      ++bin_no;
      if (bin_no == 4)
        bin_no = 1;
    }
    filestrm.close();
    focusfile.remove();
    AnalysisDataService::Instance().remove("test_in");
    AnalysisDataService::Instance().remove("test_in_1");
    AnalysisDataService::Instance().remove("test_in_2");
    AnalysisDataService::Instance().remove("test_in_3");
    AnalysisDataService::Instance().remove("test_in_4");
  }

  void testSaveGSSWorkspaceGroups() {
    using namespace Mantid::API;
    using namespace Mantid::DataObjects;
    Workspace2D_sptr workspace = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 2.0);
    workspace->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");

    Workspace2D_sptr work_in1 = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 2.0);
    work_in1->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");

    Workspace2D_sptr work_in2 = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 2.0);
    work_in2->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");

    Workspace2D_sptr work_in3 = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 2.0);
    work_in3->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");

    Workspace2D_sptr work_in4 = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 2.0);
    work_in4->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");

    WorkspaceGroup_sptr wsSptr = WorkspaceGroup_sptr(new WorkspaceGroup);
    if (wsSptr) {
      AnalysisDataService::Instance().add("test_in", wsSptr);
      AnalysisDataService::Instance().add("test_in_1", work_in1);
      wsSptr->add("test_in_1");
      AnalysisDataService::Instance().add("test_in_2", work_in2);
      wsSptr->add("test_in_2");
      AnalysisDataService::Instance().add("test_in_3", work_in3);
      wsSptr->add("test_in_3");
      AnalysisDataService::Instance().add("test_in_4", work_in4);
      wsSptr->add("test_in_4");
    }
    Mantid::DataHandling::SaveGSS saveGSS;
    TS_ASSERT_THROWS_NOTHING(saveGSS.initialize());
    TS_ASSERT_EQUALS(saveGSS.isInitialized(), true);

    saveGSS.setPropertyValue("InputWorkspace", "test_in");
    std::string filename("SaveGSS.txt");
    saveGSS.setPropertyValue("Filename", filename);
    filename = saveGSS.getPropertyValue("Filename"); // absolute path
    saveGSS.setProperty("SplitFiles", false);
    saveGSS.setPropertyValue("Append", "0");
    saveGSS.setPropertyValue("MultiplyByBinWidth", "1");

    TS_ASSERT_THROWS_NOTHING(saveGSS.execute());
    Poco::File focusfile(filename);
    TS_ASSERT_EQUALS(focusfile.exists(), true);

    std::ifstream filestrm(filename.c_str());
    std::string line;
    int bin_no(1);
    while (getline(filestrm, line)) {
      if (line.empty())
        continue;
      if (line[0] == '#')
        continue;
      std::string str = line.substr(0, 4);
      if (str == "BANK")
        continue;
      double x(0.0), y(0.0), e(0.);
      std::istringstream is(line);
      is >> x >> y >> e;
      switch (bin_no) {
      case 1:
        TS_ASSERT_DELTA(x, 2.0, m_tol); // center of the bin
        TS_ASSERT_DELTA(y, 4.0, m_tol); // width (2.0) * value (2.0)
        TS_ASSERT_DELTA(e, 1.41421356 * 2.0,
                        m_tol); // error (sqrt(2) * bin width (2.0)
        break;
      case 2:
        TS_ASSERT_DELTA(x, 4.0, m_tol);
        TS_ASSERT_DELTA(y, 4.0, m_tol);
        TS_ASSERT_DELTA(e, 1.41421356 * 2.0, m_tol);
        break;
      case 3:
        TS_ASSERT_DELTA(x, 6.0, m_tol);
        TS_ASSERT_DELTA(y, 4.0, m_tol);
        TS_ASSERT_DELTA(e, 1.41421356 * 2.0, m_tol);
        break;
      default:
        TS_ASSERT(false);
      }
      ++bin_no;
      if (bin_no == 4)
        bin_no = 1;
    }
    filestrm.close();
    focusfile.remove();
    AnalysisDataService::Instance().remove("test_in");
    AnalysisDataService::Instance().remove("test_in_1");
    AnalysisDataService::Instance().remove("test_in_2");
    AnalysisDataService::Instance().remove("test_in_3");
    AnalysisDataService::Instance().remove("test_in_4");
  }

  void testSaveGSSWorkspaceGroups_dont_multiply_bin_width() {
    using namespace Mantid::API;
    using namespace Mantid::DataObjects;
    Workspace2D_sptr workspace = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 2.0);
    workspace->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");

    Workspace2D_sptr work_in1 = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 2.0);
    work_in1->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");

    WorkspaceGroup_sptr wsSptr = WorkspaceGroup_sptr(new WorkspaceGroup);
    if (wsSptr) {
      AnalysisDataService::Instance().add("test_in", wsSptr);
      AnalysisDataService::Instance().add("test_in_1", work_in1);
      wsSptr->add("test_in_1");
    }
    Mantid::DataHandling::SaveGSS saveGSS;
    TS_ASSERT_THROWS_NOTHING(saveGSS.initialize());
    TS_ASSERT_EQUALS(saveGSS.isInitialized(), true);

    saveGSS.setPropertyValue("InputWorkspace", "test_in");
    std::string filename("SaveGSS.txt");
    saveGSS.setPropertyValue("Filename", filename);
    filename = saveGSS.getPropertyValue("Filename"); // absolute path
    saveGSS.setProperty("SplitFiles", false);
    saveGSS.setPropertyValue("Append", "0");
    saveGSS.setPropertyValue("MultiplyByBinWidth", "0");

    TS_ASSERT_THROWS_NOTHING(saveGSS.execute());
    Poco::File focusfile(filename);
    TS_ASSERT_EQUALS(focusfile.exists(), true);

    std::ifstream filestrm(filename.c_str());
    std::string line;
    int bin_no(1);
    while (getline(filestrm, line)) {
      if (line.empty())
        continue;
      if (line[0] == '#')
        continue;
      std::string str = line.substr(0, 4);
      if (str == "BANK")
        continue;
      double x(0.0), y(0.0), e(0.);
      std::istringstream is(line);
      is >> x >> y >> e;
      switch (bin_no) {
      case 1:
        TS_ASSERT_DELTA(x, 2.0, m_tol);              // center of the bin
        TS_ASSERT_DELTA(y, 2.0, m_tol);              // width (2.0)
        TS_ASSERT_DELTA(e, 1.41421356 * 1.0, m_tol); // error (sqrt(2)
        break;
      case 2:
        TS_ASSERT_DELTA(x, 4.0, m_tol);
        TS_ASSERT_DELTA(y, 2.0, m_tol);
        TS_ASSERT_DELTA(e, 1.41421356 * 1.0, m_tol);
        break;
      case 3:
        TS_ASSERT_DELTA(x, 6.0, m_tol);
        TS_ASSERT_DELTA(y, 2.0, m_tol);
        TS_ASSERT_DELTA(e, 1.41421356 * 1.0, m_tol);
        break;
      default:
        TS_ASSERT(false);
      }
      ++bin_no;
      if (bin_no == 4)
        bin_no = 1;
    }
    filestrm.close();
    focusfile.remove();
    AnalysisDataService::Instance().remove("test_in");
  }

  void testDistribution() {
    using namespace Mantid::API;
    using namespace Mantid::DataObjects;
    Workspace2D_sptr workspace = WorkspaceCreationHelper::create2DWorkspace154(1, 3, false);
    workspace->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("TOF");

    TS_ASSERT_DIFFERS(workspace, std::shared_ptr<Workspace2D>());
    std::string resultWS("result");
    AnalysisDataService::Instance().add(resultWS, workspace);

    Mantid::DataHandling::SaveFocusedXYE saveXYE;
    TS_ASSERT_THROWS_NOTHING(saveXYE.initialize());
    TS_ASSERT_EQUALS(saveXYE.isInitialized(), true);

    saveXYE.setPropertyValue("InputWorkspace", resultWS);
    std::string filename("focussed.test");
    saveXYE.setPropertyValue("Filename", filename);
    filename = saveXYE.getPropertyValue("Filename"); // absolute path
    saveXYE.setProperty("SplitFiles", false);

    TS_ASSERT_THROWS_NOTHING(saveXYE.execute());

    Poco::File focusfile(filename);
    TS_ASSERT_EQUALS(focusfile.exists(), true);

    std::ifstream filestrm(filename.c_str());
    std::string line;
    int bin_no(1);
    double x_value = 1.0;
    size_t lineNumber = 0;
    while (getline(filestrm, line)) {
      lineNumber++;
      if (lineNumber <= MAX_HEADER_LENGTH)
        continue;
      double x(0.0), y(0.0), e(0.);
      std::istringstream is(line);
      is >> x >> y >> e;
      switch (bin_no) {
      case 1:
      case 2:
      case 3:
        TS_ASSERT_DELTA(x, x_value, m_tol);
        TS_ASSERT_DELTA(y, 5.0, m_tol);
        TS_ASSERT_DELTA(e, 4.0, m_tol);
        break;
      default:
        TS_ASSERT(false);
      }
      x_value += 1.0;
      ++bin_no;
    }
    filestrm.close();
    focusfile.remove();
    AnalysisDataService::Instance().remove(resultWS);
  }

  void test_doesnt_fail_on_missing_detectors() {
    using namespace Mantid::API;
    using namespace Mantid::DataObjects;

    // Create workspace with full instrument and three spectra
    Workspace2D_sptr workspace = WorkspaceCreationHelper::create2DWorkspaceWithFullInstrument(3, 3);
    // Remove detectors from one spectrum
    workspace->getSpectrum(1).clearDetectorIDs();

    std::string createdWS("ws");
    AnalysisDataService::Instance().add(createdWS, workspace);

    std::string filename("focussed.test");
    Mantid::DataHandling::SaveFocusedXYE saveXYE;
    saveXYE.initialize();
    saveXYE.setPropertyValue("InputWorkspace", "ws");
    saveXYE.setPropertyValue("Filename", filename);
    filename = saveXYE.getPropertyValue("Filename"); // absolute path
    saveXYE.setProperty("SplitFiles", false);
    saveXYE.execute();
    TS_ASSERT(saveXYE.isExecuted());
    Poco::File focusfile(filename);
    TS_ASSERT_EQUALS(focusfile.exists(), true);
    if (focusfile.exists())
      focusfile.remove();
    Mantid::API::AnalysisDataService::Instance().clear();
  }

private:
  const double m_tol;
};

class SaveFocusedXYETestPerformance : public CxxTest::TestSuite {
public:
  void setUp() override {
    // Create a workspace for writing out
    Mantid::API::MatrixWorkspace_sptr dataws = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 3, 1.0, 1.0);

    Mantid::API::AnalysisDataService::Instance().addOrReplace(m_wsName, dataws);

    for (int i = 0; i < numberOfIterations; ++i) {
      saveAlgPtrs.emplace_back(setupAlg());
    }
  }

  void testSaveFocusedPerformance() {
    for (auto alg : saveAlgPtrs) {
      TS_ASSERT_THROWS_NOTHING(alg->execute());
    }
  }

  void tearDown() override {
    for (int i = 0; i < numberOfIterations; i++) {
      delete saveAlgPtrs[i];
      saveAlgPtrs[i] = nullptr;
    }
    Mantid::API::AnalysisDataService::Instance().remove(m_wsName);
    Poco::File focusedFile(m_filename);
    if (focusedFile.exists())
      focusedFile.remove();
  }

private:
  std::vector<Mantid::DataHandling::SaveFocusedXYE *> saveAlgPtrs;

  const int numberOfIterations = 5;

  const std::string m_wsName = "SaveFocusedXYETestPerformance";
  const std::string m_filename = "test_performance.txt";

  Mantid::DataHandling::SaveFocusedXYE *setupAlg() {
    Mantid::DataHandling::SaveFocusedXYE *saver = new Mantid::DataHandling::SaveFocusedXYE;
    saver->initialize();
    saver->setPropertyValue("InputWorkspace", m_wsName);
    saver->setProperty("Filename", m_filename);
    saver->setRethrows(true);
    return saver;
  }
};
