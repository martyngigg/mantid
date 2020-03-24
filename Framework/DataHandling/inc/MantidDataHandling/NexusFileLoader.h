// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2013 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidAPI/IFileLoader.h"
#include "MantidKernel/System.h"
#include "MantidNexus/NexusHDF5Descriptor.h"

namespace Mantid::DataHandling {

class DLLExport NexusFileLoader
    : public API::IFileLoader<Mantid::NeXus::NexusHDF5Descriptor> {
public:
  void exec() override;
  boost::shared_ptr<Algorithm> createChildAlgorithm(
      const std::string &name, const double startProgress = -1.,
      const double endProgress = -1., const bool enableLogging = true,
      const int &version = -1) override;
  void
  setFileInfo(std::shared_ptr<Mantid::NeXus::NexusHDF5Descriptor> fileInfo);

private:
  std::shared_ptr<Mantid::NeXus::NexusHDF5Descriptor> m_fileInfo;
};
} // namespace Mantid::DataHandling
