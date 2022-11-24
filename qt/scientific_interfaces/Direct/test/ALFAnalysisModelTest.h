// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include <cxxtest/TestSuite.h>

#include "ALFAnalysisModel.h"

#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/MatrixWorkspace_fwd.h"
#include "MantidFrameworkTestHelpers/WorkspaceCreationHelper.h"

#include <memory>
#include <string>
#include <utility>

using namespace Mantid::API;
using namespace MantidQt::CustomInterfaces;

class ALFAnalysisModelTest : public CxxTest::TestSuite {
public:
  ALFAnalysisModelTest() { FrameworkManager::Instance(); }

  static ALFAnalysisModelTest *createSuite() { return new ALFAnalysisModelTest(); }

  static void destroySuite(ALFAnalysisModelTest *suite) { delete suite; }

  void setUp() override {
    m_workspace = WorkspaceCreationHelper::create2DWorkspace(1, 100);
    m_workspaceName = "test";
    m_range = std::make_pair<double, double>(0.0, 100.0);

    AnalysisDataService::Instance().addOrReplace(m_workspaceName, m_workspace);

    m_model = std::make_unique<ALFAnalysisModel>();
  }

  void tearDown() override {
    AnalysisDataService::Instance().clear();
    m_model.reset();
  }

  void test_that_the_model_is_instantiated_with_a_function_and_empty_fit_status() {
    TS_ASSERT_THROWS_NOTHING(m_model->peakCentre());
    TS_ASSERT_EQUALS("", m_model->fitStatus());
    TS_ASSERT_EQUALS(0u, m_model->numberOfTubes());
    TS_ASSERT_EQUALS(std::nullopt, m_model->averageTwoTheta());
    TS_ASSERT(m_model->allTwoThetas().empty());
  }

  void test_that_doFit_sets_a_successful_fit_status_for_a_good_fit() {
    m_model->doFit(m_workspaceName, m_range);

    TS_ASSERT_EQUALS(0.0, m_model->peakCentre());
    TS_ASSERT_EQUALS("success", m_model->fitStatus());
  }

  void test_that_calculateEstimate_returns_zero_peak_centre_if_the_workspace_does_not_exist_in_the_ADS() {
    AnalysisDataService::Instance().clear();

    m_model->calculateEstimate(m_workspaceName, m_range);

    TS_ASSERT_EQUALS(0.0, m_model->peakCentre());
    TS_ASSERT_EQUALS("", m_model->fitStatus());
  }

  void test_that_calculateEstimate_returns_an_estimate_if_the_workspace_does_exist_in_the_ADS() {
    m_model->calculateEstimate(m_workspaceName, m_range);

    TS_ASSERT_EQUALS(0.5, m_model->peakCentre());
    TS_ASSERT_EQUALS("", m_model->fitStatus());
  }

  void test_that_calculateEstimate_returns_zero_peak_centre_if_the_crop_range_is_invalid() {
    m_workspace = WorkspaceCreationHelper::create2DWorkspaceBinned(1, 100, 300.0);
    AnalysisDataService::Instance().addOrReplace(m_workspaceName, m_workspace);

    m_model->calculateEstimate(m_workspaceName, m_range);

    TS_ASSERT_EQUALS(0.0, m_model->peakCentre());
    TS_ASSERT_EQUALS("", m_model->fitStatus());
  }

  void test_that_setPeakCentre_will_remove_the_fit_status_and_set_the_peak_centre() {
    m_model->doFit(m_workspaceName, m_range);

    m_model->setPeakCentre(1.1);

    TS_ASSERT_EQUALS(1.1, m_model->peakCentre());
    TS_ASSERT_EQUALS("", m_model->fitStatus());
  }

  void test_that_averagedTwoTheta_returns_the_average_of_the_two_thetas_in_the_model() {
    m_model->addTwoTheta(29.5);
    m_model->addTwoTheta(30.4);
    m_model->addTwoTheta(31.0);

    TS_ASSERT_EQUALS(30.3, *m_model->averageTwoTheta());

    auto const expectedTwoThetas = std::vector<double>{29.5, 30.4, 31.0};
    TS_ASSERT_EQUALS(expectedTwoThetas, m_model->allTwoThetas());
  }

  void test_that_addTwoTheta_will_add_a_two_theta_to_the_model() {
    m_model->addTwoTheta(29.5);
    TS_ASSERT_EQUALS(29.5, *m_model->averageTwoTheta());
    TS_ASSERT_EQUALS(std::vector<double>{29.5}, m_model->allTwoThetas());
  }

  void test_that_clearTwoThetas_will_clear_the_two_thetas_from_the_model() {
    m_model->addTwoTheta(29.5);
    m_model->addTwoTheta(30.4);
    m_model->addTwoTheta(31.0);

    m_model->clearTwoThetas();

    TS_ASSERT_EQUALS(std::nullopt, m_model->averageTwoTheta());
    TS_ASSERT(m_model->allTwoThetas().empty());
  }

  void test_that_numberOfTubes_returns_the_number_of_two_thetas() {
    m_model->addTwoTheta(29.5);
    m_model->addTwoTheta(30.4);

    TS_ASSERT_EQUALS(2u, m_model->numberOfTubes());
  }

private:
  MatrixWorkspace_sptr m_workspace;
  std::string m_workspaceName;
  std::pair<double, double> m_range;

  std::unique_ptr<ALFAnalysisModel> m_model;
};