#ifndef MANTIDQTCUSTOMINTERFACESIDA_MSDFIT_H_
#define MANTIDQTCUSTOMINTERFACESIDA_MSDFIT_H_

#include "IndirectFitAnalysisTab.h"
#include "MSDFitModel.h"
#include "ui_MSDFit.h"

#include "MantidAPI/IFunction.h"

namespace MantidQt {
namespace CustomInterfaces {
namespace IDA {
class DLLExport MSDFit : public IndirectFitAnalysisTab {
  Q_OBJECT

public:
  MSDFit(QWidget *parent = nullptr);

private:
  void setupFitTab() override;

protected slots:
  void plotClicked();
  void runClicked();
  void updatePlotOptions() override;
  void updateModelFitTypeString();

protected:
  bool shouldEnablePlotResult() override;
<<<<<<< HEAD

  void setRunEnabled(bool enabled) override;
=======
  bool allowPlotResultWithOneSpectra() override { return false; }

>>>>>>> Plot Result only for workspaces with multiple spectra Refs #23164
  void setPlotResultEnabled(bool enabled) override;
  void setSaveResultEnabled(bool enabled) override;

private:
  MSDFitModel *m_msdFittingModel;
  std::unique_ptr<Ui::MSDFit> m_uiForm;
};
} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt

#endif /* MANTIDQTCUSTOMINTERFACESIDA_MSDFIT_H_ */
