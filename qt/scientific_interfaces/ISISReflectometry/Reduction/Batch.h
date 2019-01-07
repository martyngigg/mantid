// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTID_CUSTOMINTERFACES_BATCH_H_
#define MANTID_CUSTOMINTERFACES_BATCH_H_

#include "Common/DllConfig.h"
#include "Experiment.h"
#include "Instrument.h"
#include "RunsTable.h"
#include "Slicing.h"

namespace MantidQt {
namespace CustomInterfaces {

class MANTIDQT_ISISREFLECTOMETRY_DLL Batch {
public:
  Batch(Experiment const &experiment, Instrument const &instrument,
        RunsTable &runsTable, Slicing const &slicing);

  Experiment const &experiment() const;
  Instrument const &instrument() const;
  RunsTable const &runsTable() const;
  RunsTable &runsTable();
  Slicing const &slicing() const;

private:
  Experiment const &m_experiment;
  Instrument const &m_instrument;
  RunsTable &m_runsTable;
  Slicing const &m_slicing;
};
} // namespace CustomInterfaces
} // namespace MantidQt
#endif // MANTID_CUSTOMINTERFACES_BATCH_H_
