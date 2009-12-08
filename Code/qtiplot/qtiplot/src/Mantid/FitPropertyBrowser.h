#ifndef FITPROPERTYBROWSER_H_
#define FITPROPERTYBROWSER_H_

#include "MantidAPI/Workspace.h"
#include "MantidAPI/AlgorithmObserver.h"
#include <boost/shared_ptr.hpp>
#include <QDockWidget>
#include <QMap>

    /* Forward declarations */

class QtTreePropertyBrowser;
class QtGroupPropertyManager;
class QtDoublePropertyManager;
class QtIntPropertyManager;
class QtBoolPropertyManager;
class QtStringPropertyManager;
class QtEnumPropertyManager;
class QtProperty;
class QtBrowserItem;

class ApplicationWindow;

namespace Mantid
{
  namespace API
  {
    class IFunction;
    class IPeakFunction;
    class CompositeFunction;
    class Workspace;
  }
}
/**
 * Class FitPropertyBrowser implements QtPropertyBrowser to display 
 * and control fitting function parameters and settings.
 * 
 * @autor Roman Tolchenov, Tessella Support Services plc
 * @date 13/11/2009
 */

class FitPropertyBrowser: public QDockWidget, public Mantid::API::AlgorithmObserver
{
  Q_OBJECT
public:
  /// Constructor
  FitPropertyBrowser(QWidget* parent);
  /// Centre of the current peak
  double centre()const;
  /// Set centre of the current peak
  void setCentre(double value);
  /// Height of the current peak
  double height()const;
  /// Set height of the current peak
  void setHeight(double value);
  /// Width of the current peak
  double width()const;
  /// Set width of the current peak
  void setWidth(double value);
  /// Get count
  int count()const;
  /// Get index
  int index()const;
  /// Set index
  void setIndex(int i)const;
  /// Is the current function a peak?
  bool isPeak()const;
  /// Get the i-th function
  Mantid::API::IFunction* function(int i)const;
  /// Get the current function
  Mantid::API::IFunction* function()const{return function(index());}
  /// Get the i-th function if it is a peak
  Mantid::API::IPeakFunction* peakFunction(int i)const;
  /// Get the current function if it is a peak
  Mantid::API::IPeakFunction* peakFunction()const{return peakFunction(index());}
  /// Select a function
  void selectFunction(int i)const;

  /// Create a new function
  void addFunction(const std::string& fnName);
  /// Replace function
  void replaceFunction(int i,const std::string& fnName);
  /// Remove function
  void removeFunction(int i);
  /// Get Composite Function
  boost::shared_ptr<Mantid::API::CompositeFunction> compositeFunction()const{return m_compositeFunction;}
  /// Get the current function type
  std::string functionType(int i)const;
  std::string functionType()const{return functionType(index());}
  /// Get the current function name
  QString functionName(int i)const;
  QString functionName()const{return functionName(index());}
  /// Get the default function name
  std::string defaultFunctionType()const;

  /// Get the input workspace name
  std::string workspaceName()const;
  /// Set the input workspace name
  void setWorkspaceName(const QString& wsName);
  /// Get workspace index
  int workspaceIndex()const;
  /// Set workspace index
  void setWorkspaceIndex(int i);
  /// Get the output name
  std::string outputName()const;
  /// Set the output name
  void setOutputName(const std::string&);

  /// Get the start X
  double startX()const;
  /// Set the start X
  void setStartX(double);
  /// Get the end X
  double endX()const;
  /// Set the end X
  void setEndX(double);

  void init();
  void reinit();

signals:
  void indexChanged(int i)const;
  void functionRemoved(int i);
  void algorithmFinished(const QString&);
  void workspaceIndexChanged(int i);
  void workspaceNameChanged(const QString&);
  void functionChanged(const QString&);
  void startXChanged(double);
  void endXChanged(double);

private slots:
  void enumChanged(QtProperty* prop);
  void boolChanged(QtProperty* prop);
  void intChanged(QtProperty* prop);
  void doubleChanged(QtProperty* prop);
  void stringChanged(QtProperty* prop);
  void fit();
  void workspace_added(const QString &, Mantid::API::Workspace_sptr);
  void workspace_removed(const QString &);
  void currentItemChanged(QtBrowserItem*);

  void popupMenu(const QPoint &);
  /* Context menu slots */
  void addFunction();
  void deleteFunction();
private:

  /// Create CompositeFunction
  void createCompositeFunction();
  /// Get and store available workspace names
  void populateWorkspaceNames();
  /// Get the registered function names
  void populateFunctionNames();
  /// Check if the workspace can be used in the fit
  bool isWorkspaceValid(Mantid::API::Workspace_sptr)const;
  /// Called when the fit is finished
  void finishHandle(const Mantid::API::IAlgorithm* alg);
  /// Find QtBrowserItem for a property prop among the chidren of 
  QtBrowserItem* findItem(QtBrowserItem* parent,QtProperty* prop)const;

  QtTreePropertyBrowser* m_browser;
  /// Property managers:
  QtGroupPropertyManager  *m_groupManager;
  QtDoublePropertyManager *m_doubleManager;
  QtStringPropertyManager *m_stringManager;
  QtEnumPropertyManager *m_enumManager;
  QtIntPropertyManager *m_intManager;
  QtBoolPropertyManager *m_boolManager;
  /// Properties:

  mutable int m_index;

  /// The top level group
  QtBrowserItem* m_fitGroup;
  /// Group for functions
  QtProperty* m_functionsGroup;
  /// Group for input/output settings
  QtProperty* m_settingsGroup;
  /// Browser items for functions
  QList<QtBrowserItem*> m_functionItems;

  QtProperty *m_workspace;
  QtProperty *m_workspaceIndex;
  QtProperty *m_startX;
  QtProperty *m_endX;
  QtProperty *m_output;

  /// A list of registered functions
  mutable QStringList m_registeredFunctions;
  /// A list of available workspaces
  mutable QStringList m_workspaceNames;

  /// A copy of the edited function
  //Mantid::API::IFunction* m_function;
  boost::shared_ptr<Mantid::API::CompositeFunction> m_compositeFunction;

  /// Default function name
  std::string m_defaultFunction;

  /// Default width for added peaks
  double m_default_width;

  /// The current function index
  int m_index_;

  /// if true the output name will be guessed every time workspace name is changeed
  bool m_guessOutputName;

  ApplicationWindow* m_appWindow;

};


#endif /*FITPROPERTYBROWSER_H_*/