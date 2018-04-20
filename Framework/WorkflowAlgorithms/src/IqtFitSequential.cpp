#include "MantidWorkflowAlgorithms/IqtFitSequential.h"

using namespace Mantid::API;

namespace {
Mantid::Kernel::Logger g_log("IqtFitSequential");

MatrixWorkspace_sptr cropWorkspace(MatrixWorkspace_sptr workspace,
                                   double startX, double endX) {
  auto cropper = AlgorithmManager::Instance().create("CropWorkspace");
  cropper->setLogging(false);
  cropper->setAlwaysStoreInADS(false);
  cropper->setProperty("InputWorkspace", workspace);
  cropper->setProperty("OutputWorkspace", "__cropped");
  cropper->setProperty("XMin", startX);
  cropper->setProperty("XMax", endX);
  cropper->execute();
  return cropper->getProperty("OutputWorkspace");
}

MatrixWorkspace_sptr convertToHistogram(MatrixWorkspace_sptr workspace) {
  auto converter = AlgorithmManager::Instance().create("ConvertToHistogram");
  converter->setLogging(false);
  converter->setAlwaysStoreInADS(false);
  converter->setProperty("InputWorkspace", workspace);
  converter->setProperty("OutputWorkspace", "__converted");
  converter->execute();
  return converter->getProperty("OutputWorkspace");
}
} // namespace

namespace Mantid {
namespace Algorithms {

using namespace API;

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(IqtFitSequential)

/// Algorithms name for identification. @see Algorithm::name
const std::string IqtFitSequential::name() const { return "IqtFitSequential"; }

/// Algorithm's version for identification. @see Algorithm::version
int IqtFitSequential::version() const { return 1; }

/// Algorithm's category for identification. @see Algorithm::category
const std::string IqtFitSequential::category() const {
  return "Workflow\\MIDAS";
}

/// Algorithm's summary for use in the GUI and help. @see Algorithm::summary
const std::string IqtFitSequential::summary() const {
  return "Fits an \\*\\_iqt file generated by I(Q, t) sequentially.";
}

/// Algorithm's see also for related algorithms. @see Algorithm::seeAlso
const std::vector<std::string> IqtFitSequential::seeAlso() const {
  return {"QENSFitSequential"};
}

std::map<std::string, std::string> IqtFitSequential::validateInputs() {
  auto errors = QENSFitSequential::validateInputs();
  double startX = getProperty("StartX");
  if (startX < 0)
    errors["StartX"] = "StartX must be greater than or equal to 0.";
  return errors;
}

bool IqtFitSequential::throwIfElasticQConversionFails() const { return true; }

std::vector<API::MatrixWorkspace_sptr> IqtFitSequential::getWorkspaces() const {
  auto workspaces = QENSFitSequential::getWorkspaces();
  double startX = getProperty("StartX");
  double endX = getProperty("EndX");

  for (auto i = 0u; i < workspaces.size(); ++i)
    workspaces[i] =
        convertToHistogram(cropWorkspace(workspaces[i], startX, endX));
  return workspaces;
}

} // namespace Algorithms
} // namespace Mantid
