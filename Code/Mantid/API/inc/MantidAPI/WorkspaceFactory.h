#ifndef MANTID_KERNEL_WORKSPACEFACTORY_H_
#define MANTID_KERNEL_WORKSPACEFACTORY_H_

/* Used to register classes into the factory. creates a global object in an
 * anonymous namespace. The object itself does nothing, but the comma operator
 * is used in the call to its constructor to effect a call to the factory's
 * subscribe method.
 */
#define DECLARE_WORKSPACE(classname) \
  namespace { \
    Mantid::Kernel::RegistrationHelper register_ws_##classname( \
       ((Mantid::API::WorkspaceFactory::Instance().subscribe<classname>(#classname)) \
       , 0)); \
  }

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/DllExport.h"
#include "MantidKernel/DynamicFactory.h"
#include "MantidKernel/SingletonHolder.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/IMDWorkspace.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidGeometry/MDGeometry/MDGeometryDescription.h"
namespace Mantid
{
namespace API
{
/** The WorkspaceFactory class is in charge of the creation of all types
    of workspaces. It inherits most of its implementation from
    the Dynamic Factory base class.
    It is implemented as a singleton class.

    @author Laurent C Chapon, ISIS, RAL
    @author Russell Taylor, Tessella Support Services plc
    @date 26/09/2007

    Copyright &copy; 2007-9 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>.
    Code Documentation is available at: <http://doxygen.mantidproject.org>
*/

class EXPORT_OPT_MANTID_API WorkspaceFactoryImpl : public Kernel::DynamicFactory<Workspace>
{
public:
  MatrixWorkspace_sptr create(const MatrixWorkspace_const_sptr& parent,
                        int NVectors = -1, int XLength = -1, int YLength = -1) const;
  MatrixWorkspace_sptr create(const std::string& className, const int& NVectors,
                                   const int& XLength, const int& YLength) const;
  
  IMDWorkspace_sptr create(const std::string & className, const Geometry::MDGeometryDescription &) const;
 /// this create method is currently used to build MD workspaces, but may be used to build MD workspaces from matrix workspaces in a future;
   IMDWorkspace_sptr create(const IMDWorkspace_sptr origin) const;

 
  ///MDWorkspaceHolder_sptr create(const IMDWorkspace_sptr origin,const MDPropertyGeometry<> &MDgeometry) const;
  //MDWorkspace_sptr create(const std::string &className="MDWorkspace",const const MDPropertyGeometry<>const *MDgeometry=NULL) const;

  void initializeFromParent(const MatrixWorkspace_const_sptr parent,
                            const MatrixWorkspace_sptr child, const bool differentSize) const;

  /// Create a ITableWorkspace
  ITableWorkspace_sptr createTable(const std::string& className = "TableWorkspace") const;

private:
  friend struct Mantid::Kernel::CreateUsingNew<WorkspaceFactoryImpl>;

  /// Private Constructor for singleton class
  WorkspaceFactoryImpl();
  /// Private copy constructor - NO COPY ALLOWED
  WorkspaceFactoryImpl(const WorkspaceFactoryImpl&);
  /// Private assignment operator - NO ASSIGNMENT ALLOWED
  WorkspaceFactoryImpl& operator = (const WorkspaceFactoryImpl&);
  ///Private Destructor
  virtual ~WorkspaceFactoryImpl();

  // Unhide the inherited create method but make it private
  using Kernel::DynamicFactory<Workspace>::create;

  /// Static reference to the logger class
  Kernel::Logger& g_log;
};

///Forward declaration of a specialisation of SingletonHolder for AlgorithmFactoryImpl (needed for dllexport/dllimport) and a typedef for it.
#ifdef _WIN32
  // this breaks new namespace declaraion rules; need to find a better fix
  template class EXPORT_OPT_MANTID_API Mantid::Kernel::SingletonHolder<WorkspaceFactoryImpl>;
#endif /* _WIN32 */
typedef EXPORT_OPT_MANTID_API Mantid::Kernel::SingletonHolder<WorkspaceFactoryImpl> WorkspaceFactory;

} // namespace Kernel
} // namespace Mantid

#endif /*MANTID_KERNEL_WORKSPACEFACTORY_H_*/
