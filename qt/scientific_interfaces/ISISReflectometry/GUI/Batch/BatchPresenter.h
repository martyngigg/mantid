// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2011 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "Common/DllConfig.h"
#include "GUI/Common/IJobRunner.h"
#include "GUI/Event/IEventPresenter.h"
#include "GUI/Experiment/IExperimentPresenter.h"
#include "GUI/Instrument/IInstrumentPresenter.h"
#include "GUI/Preview/IPreviewPresenter.h"
#include "GUI/Runs/IRunsPresenter.h"
#include "GUI/Save/ISavePresenter.h"
#include "IBatchJobManager.h"
#include "IBatchPresenter.h"
#include "MantidQtWidgets/Common/WorkspaceObserver.h"
#include <memory>

namespace MantidQt {
namespace CustomInterfaces {
namespace ISISReflectometry {

class IBatchView;

/** @class BatchPresenter

    BatchPresenter is the concrete main window presenter implementing the
    functionality defined by the interface IBatchPresenter.
*/
class MANTIDQT_ISISREFLECTOMETRY_DLL BatchPresenter : public IBatchPresenter,
                                                      public JobRunnerSubscriber,
                                                      public MantidQt::API::WorkspaceObserver {
public:
  /// Constructor
  BatchPresenter(IBatchView *view, std::unique_ptr<Batch> model, IJobRunner *jobRunner,
                 std::unique_ptr<IRunsPresenter> runsPresenter, std::unique_ptr<IEventPresenter> eventPresenter,
                 std::unique_ptr<IExperimentPresenter> experimentPresenter,
                 std::unique_ptr<IInstrumentPresenter> instrumentPresenter,
                 std::unique_ptr<ISavePresenter> savePresenter, std::unique_ptr<IPreviewPresenter> previewPresenter);
  BatchPresenter(BatchPresenter const &rhs) = delete;
  BatchPresenter(BatchPresenter &&rhs) = delete;
  BatchPresenter const &operator=(BatchPresenter const &rhs) = delete;
  BatchPresenter &operator=(BatchPresenter &&rhs) = delete;

  // JobRunnerSubscriber overrides
  void notifyBatchComplete(bool error) override;
  void notifyBatchCancelled() override;
  void notifyAlgorithmStarted(MantidQt::API::IConfiguredAlgorithm_sptr &algorithm) override;
  void notifyAlgorithmComplete(MantidQt::API::IConfiguredAlgorithm_sptr &algorithm) override;
  void notifyAlgorithmError(MantidQt::API::IConfiguredAlgorithm_sptr algorithm, std::string const &message) override;

  // IBatchPresenter overrides
  void acceptMainPresenter(IMainWindowPresenter *mainPresenter) override;
  void initInstrumentList() override;
  void notifyPauseReductionRequested() override;
  void notifyResumeReductionRequested() override;
  void notifyResumeAutoreductionRequested() override;
  void notifyPauseAutoreductionRequested() override;
  void notifyAutoreductionCompleted() override;
  void notifyChangeInstrumentRequested(const std::string &instrumentName) override;
  void notifyInstrumentChanged(const std::string &instrumentName) override;
  void notifyUpdateInstrumentRequested() override;
  void notifySettingsChanged() override;
  void notifySetRoundPrecision(int &precision) override;
  void notifyResetRoundPrecision() override;
  void notifyAnyBatchReductionResumed() override;
  void notifyAnyBatchReductionPaused() override;
  void notifyAnyBatchAutoreductionResumed() override;
  void notifyAnyBatchAutoreductionPaused() override;
  void notifyReductionPaused() override;
  bool requestClose() const override;
  bool isProcessing() const override;
  bool isAutoreducing() const override;
  bool isAnyBatchProcessing() const override;
  bool isAnyBatchAutoreducing() const override;
  bool isOverwriteBatchPrevented() const override;
  bool discardChanges(std::string const &message) const override;
  bool isBatchUnsaved() const override;
  void setBatchUnsaved() override;
  void notifyChangesSaved() override;
  Mantid::Geometry::Instrument_const_sptr instrument() const override;
  std::string instrumentName() const override;
  int percentComplete() const override;
  API::IConfiguredAlgorithm::AlgorithmRuntimeProps rowProcessingProperties() const override;

  // WorkspaceObserver overrides
  void postDeleteHandle(const std::string &wsName) override;
  void renameHandle(const std::string &oldName, const std::string &newName) override;
  void clearADSHandle() override;

private:
  bool startBatch(std::deque<MantidQt::API::IConfiguredAlgorithm_sptr> algorithms);
  void resumeReduction();
  void notifyReductionResumed();
  void pauseReduction();
  void resumeAutoreduction();
  void notifyAutoreductionResumed();
  void pauseAutoreduction();
  void notifyAutoreductionPaused();
  void autoreductionCompleted();
  void settingsChanged();

  IBatchView *m_view;
  std::unique_ptr<Batch> m_model;
  IMainWindowPresenter *m_mainPresenter;
  std::unique_ptr<IRunsPresenter> m_runsPresenter;
  std::unique_ptr<IEventPresenter> m_eventPresenter;
  std::unique_ptr<IExperimentPresenter> m_experimentPresenter;
  std::unique_ptr<IInstrumentPresenter> m_instrumentPresenter;
  std::unique_ptr<ISavePresenter> m_savePresenter;
  std::unique_ptr<IPreviewPresenter> m_previewPresenter;
  bool m_unsavedBatchFlag;
  IJobRunner *m_jobRunner;

  friend class Encoder;
  friend class Decoder;
  friend class CoderCommonTester;

protected:
  std::unique_ptr<IBatchJobManager> m_jobManager;
};
} // namespace ISISReflectometry
} // namespace CustomInterfaces
} // namespace MantidQt
