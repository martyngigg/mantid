#ifndef MANTID_CUSTOMINTERFACES_MUONANALYSISFITDATAPRESENTERTEST_H_
#define MANTID_CUSTOMINTERFACES_MUONANALYSISFITDATAPRESENTERTEST_H_

#include <cxxtest/TestSuite.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>

#include "MantidAPI/GroupingLoader.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidAPI/TableRow.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidQtCustomInterfaces/Muon/MuonAnalysisDataLoader.h"
#include "MantidQtCustomInterfaces/Muon/MuonAnalysisFitDataPresenter.h"
#include "MantidQtCustomInterfaces/Muon/MuonAnalysisHelper.h"
#include "MantidQtMantidWidgets/IMuonFitDataSelector.h"
#include "MantidQtMantidWidgets/IWorkspaceFitControl.h"

using MantidQt::CustomInterfaces::MuonAnalysisDataLoader;
using MantidQt::CustomInterfaces::MuonAnalysisFitDataPresenter;
using MantidQt::CustomInterfaces::Muon::DeadTimesType;
using MantidQt::MantidWidgets::IMuonFitDataSelector;
using MantidQt::MantidWidgets::IWorkspaceFitControl;
using Mantid::API::AnalysisDataService;
using Mantid::API::ITableWorkspace;
using Mantid::API::TableRow;
using Mantid::API::Workspace;
using Mantid::API::WorkspaceFactory;
using Mantid::API::WorkspaceGroup;
using namespace testing;

/// Mock data selector widget
class MockDataSelector : public IMuonFitDataSelector {
public:
  MOCK_CONST_METHOD0(getFilenames, QStringList());
  MOCK_CONST_METHOD0(getWorkspaceIndex, unsigned int());
  MOCK_CONST_METHOD0(getStartTime, double());
  MOCK_CONST_METHOD0(getEndTime, double());
  MOCK_METHOD1(setNumPeriods, void(size_t));
  MOCK_METHOD1(setChosenPeriod, void(const QString &));
  MOCK_CONST_METHOD0(getPeriodSelections, QStringList());
  MOCK_METHOD2(setWorkspaceDetails, void(const QString &, const QString &));
  MOCK_METHOD1(setAvailableGroups, void(const QStringList &));
  MOCK_CONST_METHOD0(getChosenGroups, QStringList());
  MOCK_METHOD1(setChosenGroup, void(const QString &));
  MOCK_METHOD1(setWorkspaceIndex, void(unsigned int));
  MOCK_METHOD1(setStartTime, void(double));
  MOCK_METHOD1(setEndTime, void(double));
  MOCK_METHOD1(setStartTimeQuietly, void(double));
  MOCK_METHOD1(setEndTimeQuietly, void(double));
  MOCK_CONST_METHOD0(getFitType, IMuonFitDataSelector::FitType());
  MOCK_CONST_METHOD0(getInstrumentName, QString());
  MOCK_CONST_METHOD0(getRuns, QString());
  QString getSimultaneousFitLabel() const override {
    return QString("UserSelectedFitLabel");
  }
  MOCK_CONST_METHOD0(getDatasetIndex, int());
  MOCK_METHOD1(setDatasetNames, void(const QStringList &));
};

/// Mock fit property browser
class MockFitBrowser : public IWorkspaceFitControl {
public:
  MOCK_METHOD1(setWorkspaceName, void(const QString &));
  MOCK_METHOD1(setStartX, void(double));
  MOCK_METHOD1(setEndX, void(double));
  MOCK_METHOD1(setWorkspaceIndex, void(int));
  MOCK_METHOD1(allowSequentialFits, void(bool));
  MOCK_METHOD1(setWorkspaceNames, void(const QStringList &));
  MOCK_METHOD1(workspacesToFitChanged, void(int));
  MOCK_METHOD1(setSimultaneousLabel, void(const std::string &));
  MOCK_METHOD1(userChangedDataset, void(int));
};

class MuonAnalysisFitDataPresenterTest : public CxxTest::TestSuite {
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static MuonAnalysisFitDataPresenterTest *createSuite() {
    return new MuonAnalysisFitDataPresenterTest();
  }
  static void destroySuite(MuonAnalysisFitDataPresenterTest *suite) {
    delete suite;
  }

  /// Constructor
  MuonAnalysisFitDataPresenterTest()
      : m_dataLoader(DeadTimesType::None,
                     QStringList{"MUSR", "EMU", "HIFI", "ARGUS", "CHRONUS"}) {
    Mantid::API::FrameworkManager::Instance();
  }

  /// Run before each test to create mock objects
  void setUp() override {
    Mantid::API::Grouping grouping;
    grouping.groupNames = {"fwd", "bwd"};
    grouping.pairNames = {"long"};
    grouping.groups = {"1-32", "33-64"};
    grouping.pairs.emplace_back(0, 1);
    grouping.pairAlphas = {1.0};
    m_dataSelector = new NiceMock<MockDataSelector>();
    m_fitBrowser = new NiceMock<MockFitBrowser>();
    m_presenter = new MuonAnalysisFitDataPresenter(
        m_fitBrowser, m_dataSelector, m_dataLoader, grouping,
        MantidQt::CustomInterfaces::Muon::PlotType::Asymmetry);
  }

  /// Run after each test to check expectations and remove mocks
  void tearDown() override {
    TS_ASSERT(Mock::VerifyAndClearExpectations(m_dataSelector));
    TS_ASSERT(Mock::VerifyAndClearExpectations(m_fitBrowser));
    delete m_dataSelector;
    delete m_fitBrowser;
    delete m_presenter;
    AnalysisDataService::Instance().clear();
  }

  void test_handleDataPropertiesChanged() {
    ON_CALL(*m_dataSelector, getWorkspaceIndex()).WillByDefault(Return(0));
    ON_CALL(*m_dataSelector, getStartTime()).WillByDefault(Return(0.3));
    ON_CALL(*m_dataSelector, getEndTime()).WillByDefault(Return(9.9));
    EXPECT_CALL(*m_fitBrowser, setWorkspaceIndex(0)).Times(1);
    EXPECT_CALL(*m_fitBrowser, setStartX(0.3)).Times(1);
    EXPECT_CALL(*m_fitBrowser, setEndX(9.9)).Times(1);
    m_presenter->handleDataPropertiesChanged();
  }

  void test_handleSelectedDataChanged_Simultaneous() {
    doTest_handleSelectedDataChanged(
        IMuonFitDataSelector::FitType::Simultaneous);
  }

  void test_handleSelectedDataChanged_CoAdd() {
    doTest_handleSelectedDataChanged(IMuonFitDataSelector::FitType::CoAdd);
  }

  void test_handleXRangeChangedGraphically() {
    EXPECT_CALL(*m_dataSelector, setStartTimeQuietly(0.4)).Times(1);
    EXPECT_CALL(*m_dataSelector, setEndTimeQuietly(9.4)).Times(1);
    m_presenter->handleXRangeChangedGraphically(0.4, 9.4);
  }

  void test_setAssignedFirstRun_singleWorkspace() {
    const QString wsName("MUSR00015189; Pair; long; Asym; 1; #1");
    EXPECT_CALL(*m_dataSelector,
                setWorkspaceDetails(QString("00015189"), QString("MUSR")))
        .Times(1);
    EXPECT_CALL(*m_dataSelector, setWorkspaceIndex(0u)).Times(1);
    EXPECT_CALL(*m_fitBrowser, allowSequentialFits(true)).Times(1);
    m_presenter->setAssignedFirstRun(wsName);
  }

  void test_setAssignedFirstRun_contiguousRange() {
    const QString wsName("MUSR00015189-91; Pair; long; Asym; 1; #1");
    EXPECT_CALL(*m_dataSelector,
                setWorkspaceDetails(QString("00015189-91"), QString("MUSR")))
        .Times(1);
    EXPECT_CALL(*m_dataSelector, setWorkspaceIndex(0u)).Times(1);
    EXPECT_CALL(*m_fitBrowser, allowSequentialFits(false)).Times(1);
    EXPECT_CALL(*m_dataSelector, setChosenGroup(QString("long"))).Times(1);
    EXPECT_CALL(*m_dataSelector, setChosenPeriod(QString("1"))).Times(1);
    m_presenter->setAssignedFirstRun(wsName);
  }

  void test_setAssignedFirstRun_nonContiguousRange() {
    const QString wsName("MUSR00015189-91, 15193; Pair; long; Asym; 1; #1");
    EXPECT_CALL(
        *m_dataSelector,
        setWorkspaceDetails(QString("00015189-91, 15193"), QString("MUSR")))
        .Times(1);
    EXPECT_CALL(*m_dataSelector, setWorkspaceIndex(0u)).Times(1);
    EXPECT_CALL(*m_fitBrowser, allowSequentialFits(false)).Times(1);
    EXPECT_CALL(*m_dataSelector, setChosenGroup(QString("long"))).Times(1);
    EXPECT_CALL(*m_dataSelector, setChosenPeriod(QString("1"))).Times(1);
    m_presenter->setAssignedFirstRun(wsName);
  }

  void test_setAssignedFirstRun_alreadySet() {
    const QString wsName("MUSR00015189; Pair; long; Asym; 1; #1");
    m_presenter->setAssignedFirstRun(wsName);
    EXPECT_CALL(*m_dataSelector, setWorkspaceDetails(_, _)).Times(0);
    EXPECT_CALL(*m_dataSelector, setWorkspaceIndex(_)).Times(0);
    EXPECT_CALL(*m_fitBrowser, allowSequentialFits(_)).Times(0);
    EXPECT_CALL(*m_dataSelector, setChosenGroup(QString("long"))).Times(0);
    EXPECT_CALL(*m_dataSelector, setChosenPeriod(QString("1"))).Times(0);
    m_presenter->setAssignedFirstRun(wsName);
  }

  void test_getAssignedFirstRun() {
    const QString wsName("MUSR00015189; Pair; long; Asym; 1; #1");
    m_presenter->setAssignedFirstRun(wsName);
    TS_ASSERT_EQUALS(wsName, m_presenter->getAssignedFirstRun());
  }

  void test_handleSimultaneousFitLabelChanged() {
    EXPECT_CALL(*m_fitBrowser,
                setSimultaneousLabel(std::string("UserSelectedFitLabel")))
        .Times(1);
    m_presenter->handleSimultaneousFitLabelChanged();
  }

  void test_handleFitFinished_nonSequential() {
    EXPECT_CALL(*m_dataSelector, getFitType())
        .Times(1)
        .WillOnce(Return(IMuonFitDataSelector::FitType::Single));
    EXPECT_CALL(*m_dataSelector, getChosenGroups())
        .Times(1)
        .WillOnce(Return(QStringList({"fwd"})));
    EXPECT_CALL(*m_dataSelector, getPeriodSelections())
        .Times(1)
        .WillOnce(Return(QStringList({"1"})));
    createFittedWorkspacesGroup(
        m_dataSelector->getSimultaneousFitLabel().toStdString(),
        {"MUSR00015189; Group; fwd; Asym; 1; #1"});
    const auto workspacesBefore =
        AnalysisDataService::Instance().getObjectNames();
    m_presenter->handleFitFinished();
    const auto workspacesAfter =
        AnalysisDataService::Instance().getObjectNames();
    // assert nothing has happened
    TS_ASSERT_EQUALS(workspacesBefore, workspacesAfter);
  }

  void test_handleFitFinished_oneRunMultiplePeriods() {
    EXPECT_CALL(*m_dataSelector, getFitType())
        .Times(1)
        .WillOnce(Return(IMuonFitDataSelector::FitType::Single));
    EXPECT_CALL(*m_dataSelector, getChosenGroups())
        .Times(1)
        .WillOnce(Return(QStringList({"fwd"})));
    EXPECT_CALL(*m_dataSelector, getPeriodSelections())
        .Times(1)
        .WillOnce(Return(QStringList({"1", "2"})));
    createFittedWorkspacesGroup(
        m_dataSelector->getSimultaneousFitLabel().toStdString(),
        {"MUSR00015189; Group; fwd; Asym; 1; #1",
         "MUSR00015189; Group; fwd; Asym; 2; #1"});
    const auto workspacesBefore =
        AnalysisDataService::Instance().getObjectNames();
    m_presenter->handleFitFinished();
    const auto workspacesAfter =
        AnalysisDataService::Instance().getObjectNames();
    // assert something has happened
    TS_ASSERT_DIFFERS(workspacesBefore, workspacesAfter);
  }

  void test_handleFitFinished_oneRunMultipleGroups() {
    EXPECT_CALL(*m_dataSelector, getFitType())
        .Times(1)
        .WillOnce(Return(IMuonFitDataSelector::FitType::CoAdd));
    EXPECT_CALL(*m_dataSelector, getChosenGroups())
        .Times(1)
        .WillOnce(Return(QStringList({"fwd", "bwd"})));
    ON_CALL(*m_dataSelector, getPeriodSelections())
        .WillByDefault(Return(QStringList({"1"})));
    createFittedWorkspacesGroup(
        m_dataSelector->getSimultaneousFitLabel().toStdString(),
        {"MUSR00015189-90; Group; fwd; Asym; 1; #1",
         "MUSR00015189-90; Group; bwd; Asym; 1; #1"});
    const auto workspacesBefore =
        AnalysisDataService::Instance().getObjectNames();
    m_presenter->handleFitFinished();
    const auto workspacesAfter =
        AnalysisDataService::Instance().getObjectNames();
    // assert something has happened
    TS_ASSERT_DIFFERS(workspacesBefore, workspacesAfter);
  }

  void test_handleFitFinished_simultaneous() {
    EXPECT_CALL(*m_dataSelector, getFitType())
        .Times(1)
        .WillOnce(Return(IMuonFitDataSelector::FitType::Simultaneous));
    ON_CALL(*m_dataSelector, getChosenGroups())
        .WillByDefault(Return(QStringList({"long"})));
    ON_CALL(*m_dataSelector, getPeriodSelections())
        .WillByDefault(Return(QStringList({"1"})));
    const auto label = m_dataSelector->getSimultaneousFitLabel().toStdString();
    const std::vector<std::string> inputNames{
        "MUSR00015189; Pair; long; Asym; 1; #1",
        "MUSR00015190; Pair; long; Asym; 1; #1"};
    createFittedWorkspacesGroup(label, inputNames);
    m_presenter->handleFitFinished();
    checkFittedWorkspacesHandledCorrectly(label, inputNames);
  }

  void test_handleDatasetIndexChanged() {
    const int index = 2;
    EXPECT_CALL(*m_fitBrowser, userChangedDataset(index)).Times(1);
    m_presenter->handleDatasetIndexChanged(index);
  }

  void test_generateWorkspaceNames_CoAdd() {
    EXPECT_CALL(*m_dataSelector, getChosenGroups())
        .Times(1)
        .WillOnce(Return(QStringList({"long"})));
    EXPECT_CALL(*m_dataSelector, getPeriodSelections())
        .Times(1)
        .WillOnce(Return(QStringList({"1"})));
    EXPECT_CALL(*m_dataSelector, getFitType())
        .Times(1)
        .WillOnce(Return(IMuonFitDataSelector::FitType::CoAdd));
    auto names = m_presenter->generateWorkspaceNames("MUSR", "15189-91", true);
    std::vector<std::string> expectedNames{
        "MUSR00015189-91; Pair; long; Asym; 1; #1"};
    TS_ASSERT_EQUALS(names, expectedNames)
  }

  void test_generateWorkspaceNames_Simultaneous() {
    EXPECT_CALL(*m_dataSelector, getChosenGroups())
        .Times(1)
        .WillOnce(Return(QStringList({"long"})));
    EXPECT_CALL(*m_dataSelector, getPeriodSelections())
        .Times(1)
        .WillOnce(Return(QStringList({"1"})));
    EXPECT_CALL(*m_dataSelector, getFitType())
        .Times(1)
        .WillOnce(Return(IMuonFitDataSelector::FitType::Simultaneous));
    auto names = m_presenter->generateWorkspaceNames("MUSR", "15189-91", true);
    std::vector<std::string> expectedNames{
        "MUSR00015189; Pair; long; Asym; 1; #1",
        "MUSR00015190; Pair; long; Asym; 1; #1",
        "MUSR00015191; Pair; long; Asym; 1; #1"};
    std::sort(names.begin(), names.end());
    TS_ASSERT_EQUALS(names, expectedNames)
  }

  void test_createWorkspacesToFit_AlreadyExists() {
    // Put workspace into ADS under this name
    auto &ads = AnalysisDataService::Instance();
    const std::vector<std::string> names{
        "MUSR00015189; Pair; long; Asym; 1; #1"};
    const auto ws = WorkspaceFactory::Instance().create("Workspace2D", 1, 1, 1);
    ads.add(names[0], ws);
    m_presenter->createWorkspacesToFit(names);
    // Ensure workspace has not been replaced in ADS
    const auto retrievedWS =
        ads.retrieveWS<Mantid::API::MatrixWorkspace>(names[0]);
    TS_ASSERT(retrievedWS);
    TS_ASSERT(Mantid::API::equals(retrievedWS, ws));
  }

  void test_createWorkspacesToFit() {
    ON_CALL(*m_dataSelector, getStartTime()).WillByDefault(Return(0.1));
    ON_CALL(*m_dataSelector, getEndTime()).WillByDefault(Return(9.9));
    auto &ads = AnalysisDataService::Instance();
    const std::vector<std::string> names{
        "MUSR00015189; Pair; long; Asym; 1; #1",
        "MUSR00015189; Group; fwd; Asym; 1; #1"};
    m_presenter->createWorkspacesToFit(names);
    // Make sure workspaces have been created and grouped together
    const auto group = ads.retrieveWS<WorkspaceGroup>("MUSR00015189");
    TS_ASSERT(group);
    for (const auto &name : names) {
      TS_ASSERT(group->contains(name));
    }
  }

private:
  void doTest_handleSelectedDataChanged(IMuonFitDataSelector::FitType fitType) {
    auto &ads = AnalysisDataService::Instance();
    EXPECT_CALL(*m_dataSelector, getInstrumentName())
        .Times(1)
        .WillOnce(Return("MUSR"));
    EXPECT_CALL(*m_dataSelector, getRuns())
        .Times(1)
        .WillOnce(Return("15189-91"));
    EXPECT_CALL(*m_dataSelector, getChosenGroups())
        .Times(1)
        .WillOnce(Return(QStringList({"fwd", "long"})));
    EXPECT_CALL(*m_dataSelector, getPeriodSelections())
        .Times(1)
        .WillOnce(Return(QStringList({"1", "1-2"})));
    EXPECT_CALL(*m_dataSelector, getFitType())
        .Times(1)
        .WillOnce(Return(fitType));
    ON_CALL(*m_dataSelector, getStartTime()).WillByDefault(Return(0.55));
    ON_CALL(*m_dataSelector, getEndTime()).WillByDefault(Return(10.0));
    const std::vector<QString> expectedNames = [&fitType]() {
      if (fitType == IMuonFitDataSelector::FitType::CoAdd) {
        return std::vector<QString>{
            "MUSR00015189-91; Group; fwd; Asym; 1; #1",
            "MUSR00015189-91; Pair; long; Asym; 1; #1",
            "MUSR00015189-91; Group; fwd; Asym; 1-2; #1",
            "MUSR00015189-91; Pair; long; Asym; 1-2; #1"};
      } else {
        return std::vector<QString>{"MUSR00015189; Group; fwd; Asym; 1; #1",
                                    "MUSR00015189; Pair; long; Asym; 1; #1",
                                    "MUSR00015189; Group; fwd; Asym; 1-2; #1",
                                    "MUSR00015189; Pair; long; Asym; 1-2; #1",
                                    "MUSR00015190; Group; fwd; Asym; 1; #1",
                                    "MUSR00015190; Pair; long; Asym; 1; #1",
                                    "MUSR00015190; Group; fwd; Asym; 1-2; #1",
                                    "MUSR00015190; Pair; long; Asym; 1-2; #1",
                                    "MUSR00015191; Group; fwd; Asym; 1; #1",
                                    "MUSR00015191; Pair; long; Asym; 1; #1",
                                    "MUSR00015191; Group; fwd; Asym; 1-2; #1",
                                    "MUSR00015191; Pair; long; Asym; 1-2; #1"};
      }
    }();
    EXPECT_CALL(*m_fitBrowser,
                setWorkspaceNames(UnorderedElementsAreArray(expectedNames)))
        .Times(1);
    EXPECT_CALL(*m_dataSelector,
                setDatasetNames(UnorderedElementsAreArray(expectedNames)))
        .Times(1);
    EXPECT_CALL(*m_fitBrowser, setWorkspaceName(_)).Times(1);
    m_dataLoader.setDeadTimesType(DeadTimesType::FromFile);
    ads.add("MUSR00015189", boost::make_shared<WorkspaceGroup>());
    m_presenter->handleSelectedDataChanged(true);
    // test that all expected names are in the ADS
    const auto namesInADS = ads.getObjectNames();
    for (const QString &name : expectedNames) {
      TS_ASSERT(namesInADS.find(name.toStdString()) != namesInADS.end());
    }
    // test that workspaces have been added to correct groups
    auto existingGroup = ads.retrieveWS<WorkspaceGroup>("MUSR00015189");
    TS_ASSERT(existingGroup);
    // Simultaneous case
    if (fitType == IMuonFitDataSelector::FitType::Simultaneous) {
      if (existingGroup) {
        for (int i = 0; i < 4; i++) {
          TS_ASSERT(existingGroup->contains(expectedNames[i].toStdString()));
        }
      }
      auto newGroup = ads.retrieveWS<WorkspaceGroup>("MUSR00015190");
      TS_ASSERT(newGroup);
      if (newGroup) {
        for (int i = 4; i < 8; i++) {
          TS_ASSERT(newGroup->contains(expectedNames[i].toStdString()));
        }
      }
    } else {
      // Coadd case
      auto newGroup = ads.retrieveWS<WorkspaceGroup>("MUSR00015189-91");
      TS_ASSERT(newGroup);
      if (newGroup) {
        for (const auto &name : expectedNames) {
          TS_ASSERT(newGroup->contains(name.toStdString()));
        }
      }
    }
  }

  /**
   * Creates a group of workspaces that are the output of a simultaneous fit,
   * that handleFitFinished() will act on, e.g.:
   *
   * MuonSimulFit_Label
   *   \__MuonSimulFit_Label_Parameters
   *   \__MuonSimulFit_Label_Workspaces
   *         \__MuonSimulFit_Label_Workspace0
   *         \__MuonSimulFit_Label_Workspace1
   *         \__ ...
   *
   * @param label :: [input] Selected label for fit
   * @param inputNames :: [input] Names of input workspaces
   */
  void createFittedWorkspacesGroup(const std::string &label,
                                   const std::vector<std::string> &inputNames) {
    auto &ads = AnalysisDataService::Instance();
    auto &wsf = WorkspaceFactory::Instance();
    const std::string baseName = "MuonSimulFit_" + label;
    const std::string groupName = baseName + "_Workspaces";
    const std::string paramName = baseName + "_Parameters";
    const std::string ncmName = baseName + "_NormalisedCovarianceMatrix";
    ads.add(baseName, boost::make_shared<WorkspaceGroup>());
    ads.add(groupName, boost::make_shared<WorkspaceGroup>());
    ads.addToGroup(baseName, groupName);
    auto paramTable = wsf.createTable();
    paramTable->addColumn("str", "Name");
    paramTable->addColumn("double", "Value");
    paramTable->addColumn("double", "Error");
    for (size_t i = 0; i < inputNames.size(); i++) {
      const std::string name = baseName + "_Workspace" + std::to_string(i);
      const auto matrixWs = wsf.create("Workspace2D", 1, 1, 1);
      const auto ws = boost::dynamic_pointer_cast<Workspace>(matrixWs);
      ads.add(name, ws);
      ads.addToGroup(groupName, name);
      TableRow rowA0 = paramTable->appendRow();
      TableRow rowA1 = paramTable->appendRow();
      rowA0 << "f" + std::to_string(i) + ".A0" << 0.1 << 0.01;
      rowA1 << "f" + std::to_string(i) + ".A1" << 0.2 << 0.02;
    }
    TableRow costFuncRow = paramTable->appendRow();
    costFuncRow << "Cost function value" << 1.0 << 0.0;
    for (size_t i = 0; i < inputNames.size(); i++) {
      TableRow row = paramTable->appendRow();
      std::ostringstream oss;
      oss << "f" << std::to_string(i) << "=" << inputNames[i];
      row << oss.str() << 0.0 << 0.0;
    }
    ads.add(paramName, paramTable);
    ads.addToGroup(baseName, paramName);
    const auto ncmWs = boost::dynamic_pointer_cast<Workspace>(
        wsf.create("Workspace2D", 1, 1, 1));
    ads.add(ncmName, ncmWs);
    ads.addToGroup(baseName, ncmName);
  }

  /**
   * Checks the results of handleFitFinished() to see if workspaces are dealt
   * with correctly:
   *
   * MuonSimulFit_Label
   *   \__MuonSimulFit_Label_MUSR00015189_long_1_Workspace
   *   \__...
   *   \__MuonSimulFit_Label_MUSR00015189_long_1_Parameters
   *   \__...
   *
   * @param label :: [input] Selected label for fit
   * @param inputNames :: [input] Names of input workspaces
   */
  void checkFittedWorkspacesHandledCorrectly(
      const std::string &label, const std::vector<std::string> &inputNames) {
    auto &ads = AnalysisDataService::Instance();

    const std::string baseName = "MuonSimulFit_" + label;
    const auto baseGroup = ads.retrieveWS<WorkspaceGroup>(baseName);
    TS_ASSERT(baseGroup);
    if (baseGroup) {
      // generate expected names
      std::vector<std::string> expectedNames{baseName +
                                             "_NormalisedCovarianceMatrix"};
      for (const auto &name : inputNames) {
        std::ostringstream oss;
        const auto wsParams =
            MantidQt::CustomInterfaces::MuonAnalysisHelper::parseWorkspaceName(
                name);
        oss << baseName << "_" << wsParams.label << "_" << wsParams.itemName
            << "_" << wsParams.periods;
        expectedNames.push_back(oss.str() + "_Workspace");
        expectedNames.push_back(oss.str() + "_Parameters");
      }
      // Check expected workspaces in group
      auto groupNames(baseGroup->getNames());
      std::sort(groupNames.begin(), groupNames.end());
      std::sort(expectedNames.begin(), expectedNames.end());
      TS_ASSERT_EQUALS(expectedNames, groupNames);

      // Check parameter tables
      for (size_t i = 0; i < baseGroup->size(); i++) {
        const auto table =
            boost::dynamic_pointer_cast<ITableWorkspace>(baseGroup->getItem(i));
        if (table) {
          auto columns = table->getColumnNames();
          std::sort(columns.begin(), columns.end());
          TS_ASSERT_EQUALS(
              columns, std::vector<std::string>({"Error", "Name", "Value"}));
          TS_ASSERT_EQUALS(table->rowCount(), 3);
          TS_ASSERT_EQUALS(table->String(0, 0), "A0");
          TS_ASSERT_EQUALS(table->String(1, 0), "A1");
          TS_ASSERT_EQUALS(table->String(2, 0), "Cost function value");
          TS_ASSERT_EQUALS(table->Double(0, 1), 0.1);
          TS_ASSERT_EQUALS(table->Double(1, 1), 0.2);
          TS_ASSERT_EQUALS(table->Double(2, 1), 1.0);
          TS_ASSERT_EQUALS(table->Double(0, 2), 0.01);
          TS_ASSERT_EQUALS(table->Double(1, 2), 0.02);
          TS_ASSERT_EQUALS(table->Double(2, 2), 0.0);
        }
      }
    }
  }

  MockDataSelector *m_dataSelector;
  MockFitBrowser *m_fitBrowser;
  MuonAnalysisFitDataPresenter *m_presenter;
  MuonAnalysisDataLoader m_dataLoader;
};

#endif /* MANTID_CUSTOMINTERFACES_MUONANALYSISFITDATAPRESENTERTEST_H_ */