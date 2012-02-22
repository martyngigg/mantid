#ifndef MANTID_DATAHANDLING_LOADLIVEDATA_H_
#define MANTID_DATAHANDLING_LOADLIVEDATA_H_

#include "MantidKernel/System.h"
#include "MantidAPI/Algorithm.h"
#include "MantidDataHandling/LiveDataAlgorithm.h"
#include "MantidAPI/MatrixWorkspace.h"

namespace Mantid
{
namespace DataHandling
{

  /** Algorithm to load a chunk of live data.
   * Called by StartLiveData and MonitorLiveData
    
    @date 2012-02-16

    Copyright &copy; 2012 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

    This file is part of Mantid.

    Mantid is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Mantid is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>
    Code Documentation is available at: <http://doxygen.mantidproject.org>
  */
  class DLLExport LoadLiveData  : public LiveDataAlgorithm
  {
  public:
    LoadLiveData();
    virtual ~LoadLiveData();
    
    virtual const std::string name() const;
    virtual int version() const;

  private:
    virtual void initDocs();
    void init();
    void exec();

    Mantid::API::Workspace_sptr processChunk(Mantid::API::MatrixWorkspace_sptr chunkWS);

    void replaceChunk(Mantid::API::Workspace_sptr chunkWS);
    void addChunk(Mantid::API::Workspace_sptr chunkWS);
    void conjoinChunk(Mantid::API::Workspace_sptr chunkWS);

    void runPostProcessing();

    /// The "accumulation" workspace = after adding, but before post-processing
    Mantid::API::Workspace_sptr m_accumWS;

    /// The final output = the post-processed accumulation workspace
    Mantid::API::Workspace_sptr m_outputWS;

  };


} // namespace DataHandling
} // namespace Mantid

#endif  /* MANTID_DATAHANDLING_LOADLIVEDATA_H_ */
