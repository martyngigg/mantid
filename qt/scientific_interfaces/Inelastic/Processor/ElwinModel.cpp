// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2022 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#include "ElwinModel.h"
#include "MantidQtWidgets/Common/WorkspaceUtils.h"
#include "MantidQtWidgets/Spectroscopy/DataValidationHelper.h"

#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/AlgorithmRuntimeProps.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidQtWidgets/Common/UserInputValidator.h"

#include <MantidAPI/AlgorithmProperties.h>
#include <MantidQtWidgets/Common/ConfiguredAlgorithm.h>
#include <QDoubleValidator>
#include <QFileInfo>

using namespace DataValidationHelper;
using namespace Mantid::API;
using namespace MantidQt::MantidWidgets;

namespace MantidQt::CustomInterfaces {

ElwinModel::ElwinModel()
    : m_integrationStart(), m_integrationEnd(), m_backgroundStart(), m_backgroundEnd(), m_backgroundSubtraction(),
      m_normalise(), m_outputWorkspaceNames() {}

API::IConfiguredAlgorithm_sptr ElwinModel::setupLoadAlgorithm(std::string const &filepath,
                                                              std::string const &outputName) const {
  auto loadAlg = AlgorithmManager::Instance().create("LoadNexus");
  loadAlg->initialize();
  loadAlg->setProperty("Filename", filepath);
  loadAlg->setProperty("OutputWorkspace", outputName);
  auto runtimeProps = std::make_unique<AlgorithmRuntimeProps>();
  API::IConfiguredAlgorithm_sptr loadAlgo =
      std::make_shared<API::ConfiguredAlgorithm>(loadAlg, std::move(runtimeProps));
  return loadAlgo;
}

API::IConfiguredAlgorithm_sptr ElwinModel::setupGroupAlgorithm(std::string const &inputWorkspacesString,
                                                               std::string const &inputGroupWsName) const {
  auto groupWsAlg = AlgorithmManager::Instance().create("GroupWorkspaces");
  auto runtimeProps = std::make_unique<Mantid::API::AlgorithmRuntimeProps>();
  runtimeProps->setPropertyValue("InputWorkspaces", inputWorkspacesString);
  runtimeProps->setPropertyValue("OutputWorkspace", inputGroupWsName);
  MantidQt::API::IConfiguredAlgorithm_sptr groupAlg =
      std::make_shared<API::ConfiguredAlgorithm>(groupWsAlg, std::move(runtimeProps));
  return groupAlg;
}

API::IConfiguredAlgorithm_sptr ElwinModel::setupElasticWindowMultiple(std::string const &workspaceBaseName,
                                                                      std::string const &inputGroupWsName,
                                                                      std::string const &sampleEnvironmentLogName,
                                                                      std::string const &sampleEnvironmentLogValue) {

  setOutputWorkspaceNames(workspaceBaseName);

  // Configure ElasticWindowMultiple algorithm
  auto elwinMultAlg = AlgorithmManager::Instance().create("ElasticWindowMultiple");
  elwinMultAlg->initialize();
  auto properties = std::make_unique<Mantid::API::AlgorithmRuntimeProps>();
  properties->setProperty("OutputInQ", m_outputWorkspaceNames["qWorkspace"]);
  properties->setProperty("OutputInQSquared", m_outputWorkspaceNames["qSquaredWorkspace"]);
  properties->setProperty("OutputELF", m_outputWorkspaceNames["elfWorkspace"]);

  properties->setProperty("SampleEnvironmentLogName", sampleEnvironmentLogName);
  properties->setProperty("SampleEnvironmentLogValue", sampleEnvironmentLogValue);

  properties->setProperty("IntegrationRangeStart", m_integrationStart);
  properties->setProperty("IntegrationRangeEnd", m_integrationEnd);

  if (m_backgroundSubtraction) {
    properties->setProperty("BackgroundRangeStart", m_backgroundStart);
    properties->setProperty("BackgroundRangeEnd", m_backgroundEnd);
  }

  if (m_normalise) {
    properties->setProperty("OutputELT", m_outputWorkspaceNames["eltWorkspace"]);
  }

  properties->setProperty("InputWorkspaces", inputGroupWsName);
  MantidQt::API::IConfiguredAlgorithm_sptr elwinAlg =
      std::make_shared<API::ConfiguredAlgorithm>(elwinMultAlg, std::move(properties));
  return elwinAlg;
}

void ElwinModel::ungroupAlgorithm(std::string const &inputWorkspace) const {
  auto ungroupAlg = AlgorithmManager::Instance().create("UnGroupWorkspace");
  ungroupAlg->initialize();
  ungroupAlg->setProperty("InputWorkspace", inputWorkspace);
  ungroupAlg->execute();
}

void ElwinModel::groupAlgorithm(std::string const &inputWorkspaces, std::string const &outputWorkspace) const {
  auto groupAlg = AlgorithmManager::Instance().create("GroupWorkspaces");
  groupAlg->initialize();
  groupAlg->setProperty("InputWorkspaces", inputWorkspaces);
  groupAlg->setProperty("OutputWorkspace", outputWorkspace);
  groupAlg->execute();
}

API::IConfiguredAlgorithm_sptr ElwinModel::setupExtractSpectra(MatrixWorkspace_sptr workspace,
                                                               FunctionModelSpectra const &spectra,
                                                               std::string const &outputName) const {
  std::string workspaceIndexList = "";
  for (auto const &spec : spectra) {
    workspaceIndexList += std::to_string(spec.value) + ",";
  }

  auto extractSpectra = AlgorithmManager::Instance().create("ExtractSpectra");
  auto runtimeProps = std::make_unique<Mantid::API::AlgorithmRuntimeProps>();
  runtimeProps->setProperty<MatrixWorkspace_sptr>("InputWorkspace", workspace);
  runtimeProps->setProperty("WorkspaceIndexList", workspaceIndexList);
  runtimeProps->setProperty("OutputWorkspace", outputName);

  return std::make_shared<API::ConfiguredAlgorithm>(extractSpectra, std::move(runtimeProps));
}

void ElwinModel::setOutputWorkspaceNames(std::string const &workspaceBaseName) {
  auto elwinSuffix = "_elwin_";
  m_outputWorkspaceNames["qWorkspace"] = workspaceBaseName + elwinSuffix + "eq";
  m_outputWorkspaceNames["qSquaredWorkspace"] = workspaceBaseName + elwinSuffix + "eq2";
  m_outputWorkspaceNames["elfWorkspace"] = workspaceBaseName + elwinSuffix + "elf";
  m_outputWorkspaceNames["eltWorkspace"] = workspaceBaseName + elwinSuffix + "elt";
}

std::string ElwinModel::getOutputWorkspaceNames() const {
  std::vector<std::string> keys = {"qWorkspace", "qSquaredWorkspace", "elfWorkspace", "eltWorkspace"};
  std::ostringstream oss;
  std::transform(keys.cbegin(), keys.cend(), std::ostream_iterator<std::string>(oss, ","),
                 [&](const auto &key) { return m_outputWorkspaceNames.at(key); });
  std::string outputWorkspaceNames = oss.str();
  outputWorkspaceNames.resize(outputWorkspaceNames.size() - 1);
  return outputWorkspaceNames;
}

void ElwinModel::setIntegrationStart(double integrationStart) { m_integrationStart = integrationStart; }

void ElwinModel::setIntegrationEnd(double integrationEnd) { m_integrationEnd = integrationEnd; }

void ElwinModel::setBackgroundStart(double backgroundStart) { m_backgroundStart = backgroundStart; }

void ElwinModel::setBackgroundEnd(double backgroundEnd) { m_backgroundEnd = backgroundEnd; }

void ElwinModel::setBackgroundSubtraction(bool backgroundSubtraction) {
  m_backgroundSubtraction = backgroundSubtraction;
}

void ElwinModel::setNormalise(bool normalise) { m_normalise = normalise; }

} // namespace MantidQt::CustomInterfaces
