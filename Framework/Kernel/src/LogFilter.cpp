// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidKernel/LogFilter.h"
#include "MantidKernel/FilteredTimeSeriesProperty.h"
#include "MantidKernel/TimeROI.h"
#include "MantidKernel/TimeSeriesProperty.h"

namespace Mantid::Kernel {
/**
 * Constructor taking a reference to a filter. Note that constructing a
 * LogFilter this
 * way only allows filters to be combined. They will not affect a property
 * @param filter :: A reference to a TimeSeriesProperty<bool> that will act as a
 * filter
 */
LogFilter::LogFilter(const TimeSeriesProperty<bool> &filter) : m_prop(), m_filter() { addFilter(filter); }

/**
 * Constructor
 * @param prop :: Pointer to property to be filtered. Its actual type must be
 * TimeSeriesProperty<double>
 */
LogFilter::LogFilter(const Property *prop) : m_prop(), m_filter() { m_prop.reset(convertToTimeSeriesOfDouble(prop)); }

/**
 * / Constructor from a TimeSeriesProperty<double> object to avoid overhead of
 * casts
 */
LogFilter::LogFilter(const FilteredTimeSeriesProperty<double> *timeSeries) : m_prop(), m_filter() {
  m_prop.reset(timeSeries->clone());
}

/**
 * Filter using a TimeSeriesProperty<bool>. True values mark the allowed time
 * intervals.
 * The object is cloned
 * @param filter :: Filtering mask
 */
void LogFilter::addFilter(const TimeSeriesProperty<bool> &filter) {
  if (filter.size() == 0)
    return; // nothing to do

  // clear the filter first
  if (m_prop) {
    m_prop->clearFilter();
  }

  if (!m_filter || m_filter->size() == 0) {
    m_filter.reset(filter.clone());
  } else {
    // determine if the current version of things are open-ended
    const bool isOpen(m_filter->lastValue() && filter.lastValue());
    DateAndTime firstTime = std::min(m_filter->firstTime(), filter.firstTime());
    if (m_prop && (m_prop->size() > 0)) {
      firstTime = std::min(firstTime, m_prop->firstTime());
    }

    // create a local copy of both filters to add extra values to
    auto filter1 = m_filter.get();
    auto filter2 = std::unique_ptr<TimeSeriesProperty<bool>>(filter.clone());

    TimeInterval time1 = filter1->nthInterval(filter1->size() - 1);
    TimeInterval time2 = filter2->nthInterval(filter2->size() - 1);

    if (time1.start() < time2.start()) {
      filter1->addValue(time2.start(),
                        true); // should be f1->lastValue, but it doesnt
                               // matter for boolean AND
    } else if (time2.start() < time1.start()) {
      filter2->addValue(time1.start(), true);
    }

    // temporary objects to handle the intersection calculation
    TimeROI mine(filter1);
    TimeROI theirs(filter2.get());
    mine.update_intersection(theirs);

    // put the results into the TimeSeriesProperty
    std::vector<bool> values;
    std::vector<DateAndTime> times;
    for (const auto splitter : mine.toSplitters()) {
      values.emplace_back(true);
      values.emplace_back(false);
      times.emplace_back(splitter.start());
      times.emplace_back(splitter.stop());
    }
    // if both are open ended, remove the ending
    if (isOpen) {
      times.pop_back();
      values.pop_back();
    }

    // set as the filter
    m_filter->replaceValues(times, values);
  }
  // apply the filter to the property
  if (m_prop) {
    m_prop->filterWith(m_filter.get());
  }
}

//-------------------------------------------------------------------------------------------------
/// Clears filters
void LogFilter::clear() {
  if (m_prop)
    m_prop->clearFilter();
  m_filter.reset();
}

//--------------------------------------------------------------------------------------------------
// Private methods
//--------------------------------------------------------------------------------------------------
namespace {
template <typename SrcType> struct ConvertToTimeSeriesDouble {
  static FilteredTimeSeriesProperty<double> *apply(const Property *prop) {
    auto srcTypeSeries = dynamic_cast<const FilteredTimeSeriesProperty<SrcType> *>(prop);
    if (!srcTypeSeries)
      return nullptr;
    auto converted = new FilteredTimeSeriesProperty<double>(prop->name());
    auto pmap = srcTypeSeries->valueAsMap();
    for (auto it = pmap.begin(); it != pmap.end(); ++it) {
      converted->addValue(it->first, double(it->second));
    }
    return converted;
  }
};

/// Specialization for a double so that it just clones the input
template <> struct ConvertToTimeSeriesDouble<double> {
  static FilteredTimeSeriesProperty<double> *apply(const Property *prop) {
    auto doubleSeries = dynamic_cast<const FilteredTimeSeriesProperty<double> *>(prop);
    if (!doubleSeries)
      return nullptr;
    return doubleSeries->clone();
  }
};
} // namespace

/**
 * Converts the given property to a TimeSeriesProperty<double>, throws if
 * invalid.
 * @param prop :: A pointer to a property
 * @return A new TimeSeriesProperty<double> object converted from the input.
 * Throws std::invalid_argument if not possible
 */
FilteredTimeSeriesProperty<double> *LogFilter::convertToTimeSeriesOfDouble(const Property *prop) {
  if (auto doubleSeries = ConvertToTimeSeriesDouble<double>::apply(prop)) {
    return doubleSeries;
  } else if (auto doubleSeries = ConvertToTimeSeriesDouble<int>::apply(prop)) {
    return doubleSeries;
  } else if (auto doubleSeries = ConvertToTimeSeriesDouble<bool>::apply(prop)) {
    return doubleSeries;
  } else {
    throw std::invalid_argument("LogFilter::convertToTimeSeriesOfDouble - Cannot convert property, \"" + prop->name() +
                                "\", to double series.");
  }
}

} // namespace Mantid::Kernel
