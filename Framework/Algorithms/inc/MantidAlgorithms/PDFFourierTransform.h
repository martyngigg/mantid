#ifndef MANTID_ALGORITHMS_PDFFourierTransform_H_
#define MANTID_ALGORITHMS_PDFFourierTransform_H_

#include "MantidKernel/System.h"
#include "MantidAPI/Algorithm.h"

namespace Mantid {
namespace Algorithms {

/** PDFFourierTransform : TODO: DESCRIPTION
 */
class DLLExport PDFFourierTransform : public API::Algorithm {
public:
  PDFFourierTransform();
  ~PDFFourierTransform() override;

  /// Algorithm's name for identification
  const std::string name() const override;
  /// Summary of algorithms purpose
  const std::string summary() const override {
    return "Fourier transform from S(Q) to G(r), which is paired distribution "
           "function (PDF). G(r) will be stored in another named workspace.";
  }

  /// Algorithm's version for identification
  int version() const override;
  /// Algorithm's category for identification
  const std::string category() const override;
  /// @copydoc Algorithm::validateInputs()
  std::map<std::string, std::string> validateInputs() override;

private:
  /// Initialize the properties
  void init() override;
  /// Run the algorithm
  void exec() override;
};

} // namespace Mantid
} // namespace Algorithms

#endif /* MANTID_ALGORITHMS_PDFFourierTransform_H_ */
