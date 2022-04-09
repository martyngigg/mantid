// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2022 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#define _GNU_SOURCE
#include <boost/stacktrace.hpp>
#include <memory>
#include <vector>

namespace Mantid::Kernel {

/**
 * A debug tool for tracking life times of created shared_ptrs
 */
template <typename PointeeType> class SharedPtrTracker {
public:
  using SharedPtr = std::shared_ptr<PointeeType>;
  using WeakPtr = std::weak_ptr<PointeeType>;

  /**
   * Add a weak reference to the given shared_ptr along with the
   * stacktrace associated with its creation
   * @param object Shared_ptr to track
   * @param trace Stacktrace of creation
   */
  auto track(const SharedPtr &object, boost::stacktrace::stacktrace trace) -> void {
    m_objects.emplace_back(WeakPtr(object), std::move(trace));
  }

  /**
   * Create a string representation of live references
   * @param os Stream to write to
   * @return Live ref info
   */
  auto dumpReferencesInfo() const -> std::string {
    static const auto className = typeid(PointeeType).name();

    std::string info;
    info.append(className).append(" Lifetime Information\n").append("------------------------------\n");
    info.append("Total references tracked:  ").append(std::to_string(m_objects.size())).append("\n");
    info.append("Expired references:  ").append(std::to_string(expiredCount())).append("\n\n");

    if (totalCount() - expiredCount() > 0) {
      for (const auto &[ws, trace] : m_objects) {
        if (ws.expired())
          continue;

        auto wsShared = ws.lock();
        info.append("Active ")
            .append(className)
            .append(": Address: ")
            .append(std::string(reinterpret_cast<const char *>(wsShared.get())))
            .append(". Name: ")
            .append(wsShared->name())
            .append(". shared_ptr.use_count:  ")
            .append(std::to_string(ws.use_count()))
            .append("\n");
        info.append("Stacktrace on creation:\n").append(to_string(trace)).append("\n\n");
      }
    }

    return info;
  }

  inline auto totalCount() const -> size_t { return m_objects.size(); }
  inline auto expiredCount() const -> size_t {
    return std::count_if(m_objects.begin(), m_objects.end(),
                         [](const auto &element) { return element.workspace.expired(); });
  }

private:
  struct WeakReferenceInfo {
    WeakReferenceInfo(WeakPtr ws, boost::stacktrace::stacktrace st) noexcept
        : workspace(std::move(ws)), stackTraceAtCreation(std::move(st)) {}

    WeakPtr workspace;
    boost::stacktrace::stacktrace stackTraceAtCreation;
  };
  using WeakReferenceInfoContainer = std::vector<WeakReferenceInfo>;
  WeakReferenceInfoContainer m_objects;
};

} // namespace Mantid::Kernel