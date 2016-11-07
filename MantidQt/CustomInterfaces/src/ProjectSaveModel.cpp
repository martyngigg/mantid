#include "MantidAPI/AnalysisDataService.h"
#include "MantidQtCustomInterfaces/ProjectSaveModel.h"

using namespace Mantid::API;
using namespace MantidQt::API;
using namespace MantidQt::CustomInterfaces;

ProjectSaveModel::ProjectSaveModel(std::vector<IProjectSerialisable*> windows)
{
  auto workspaces = getWorkspaces();
  for (auto ws : workspaces) {
    std::pair<std::string, std::vector<IProjectSerialisable*>> item(ws->name(), std::vector<IProjectSerialisable*>());
    m_workspaceWindows.insert(item);
  }
}

std::vector<IProjectSerialisable *> ProjectSaveModel::getWindows(const std::string &wsName) const
{
  if(hasWindows(wsName)) {
    return m_workspaceWindows.at(wsName);
  }

  return std::vector<IProjectSerialisable*>();
}

std::set<std::string> ProjectSaveModel::getWorkspaceNames() const
{
  std::set<std::string> names;
  for(auto &item : m_workspaceWindows) {
    names.insert(item.first);
  }
  return names;
}

std::vector<Workspace_sptr> ProjectSaveModel::getWorkspaces() const {
  auto &ads = AnalysisDataService::Instance();
  return ads.getObjects();
}

bool ProjectSaveModel::hasWindows(const std::string& wsName) const
{
  auto item = m_workspaceWindows.find(wsName);
  if (item != m_workspaceWindows.end()) {
    return item->second.size() > 0;
  }

  return false;
}
