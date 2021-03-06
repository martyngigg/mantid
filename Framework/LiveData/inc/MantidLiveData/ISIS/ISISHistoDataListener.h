// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2012 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/LiveListener.h"
#include "MantidAPI/MatrixWorkspace_fwd.h"
#include "MantidKernel/cow_ptr.h"
//----------------------------------------------------------------------
// Forward declarations
//----------------------------------------------------------------------

struct idc_info;
using idc_handle_t = struct idc_info *;

namespace Mantid {
//----------------------------------------------------------------------
// Forward declarations
//----------------------------------------------------------------------

namespace HistogramData {
class BinEdges;
}

namespace LiveData {
/** ILiveListener is the interface implemented by classes which connect directly
   to
    instrument data acquisition systems (DAS) for retrieval of 'live' data into
   Mantid.
 */
class ISISHistoDataListener : public API::LiveListener {
public:
  ISISHistoDataListener();
  ~ISISHistoDataListener() override;

  std::string name() const override { return "ISISHistoDataListener"; }
  bool supportsHistory() const override { return false; }
  bool buffersEvents() const override { return false; }

  bool connect(const Poco::Net::SocketAddress &address) override;
  void start(Types::Core::DateAndTime startTime = Types::Core::DateAndTime()) override;
  std::shared_ptr<API::Workspace> extractData() override;

  bool isConnected() override;
  ILiveListener::RunStatus runStatus() override;
  int runNumber() const override;

private:
  void setSpectra(const std::vector<specnum_t> &specList) override;
  void setPeriods(const std::vector<specnum_t> &periodList);
  int getInt(const std::string &par) const;
  std::string getString(const std::string &par) const;
  void getFloatArray(const std::string &par, std::vector<float> &arr, const size_t dim);
  void getIntArray(const std::string &par, std::vector<int> &arr, const size_t dim);
  void getData(int period, int index, int count, const std::shared_ptr<API::MatrixWorkspace> &workspace,
               size_t workspaceIndex);
  void calculateIndicesForReading(std::vector<int> &index, std::vector<int> &count);
  void loadSpectraMap();
  void runLoadInstrument(const std::shared_ptr<API::MatrixWorkspace> &localWorkspace, const std::string &iName);
  void loadTimeRegimes();
  int getTimeRegimeToLoad() const;
  bool isPeriodIgnored(int period) const;
  static double dblSqrt(double in);

  /// is initialized
  bool isInitilized;

  /// the DAE name
  std::string m_daeName;

  /// the DAE handle
  idc_handle_t m_daeHandle;

  /// number of periods
  int m_numberOfPeriods;

  /// number of spectra for each time regime
  std::vector<int> m_numberOfSpectra;

  /// total number of spectra
  int m_totalNumberOfSpectra;

  /// number of bins for each time regime
  std::vector<int> m_numberOfBins;

  /// list of spectra to read or empty to read all
  std::vector<specnum_t> m_specList;

  /// list of periods to read or empty to read all
  std::vector<specnum_t> m_periodList;

  /// Store the bin boundaries for each time regime
  std::vector<HistogramData::BinEdges> m_bins;

  /// Detector IDs
  std::vector<int> m_detIDs;

  /// Spectra IDs
  std::vector<int> m_specIDs;

  /// Monitor spectra
  std::vector<int> m_monitorSpectra;

  /// Time regime to load
  int m_timeRegime;

  /// Buffer workspace to store instrument data (or not only instrument in the
  /// future), prevents loading for every chank of data
  API::MatrixWorkspace_sptr m_bufferWorkspace;

  /// reporter function called when the IDC reading routines raise an error
  static void IDCReporter(int status, int code, const char *message);
};

} // namespace LiveData
} // namespace Mantid
