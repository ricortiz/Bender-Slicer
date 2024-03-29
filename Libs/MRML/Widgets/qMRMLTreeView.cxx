/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>

// qMRML includes
#include "qMRMLItemDelegate.h"
#include "qMRMLSceneDisplayableModel.h"
#include "qMRMLSceneModelHierarchyModel.h"
#include "qMRMLSceneTransformModel.h"
#include "qMRMLSortFilterHierarchyProxyModel.h"
#include "qMRMLTreeView_p.h"

// MRML includes
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLDisplayableHierarchyNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelHierarchyLogic.h>
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLScene.h>

//------------------------------------------------------------------------------
qMRMLTreeViewPrivate::qMRMLTreeViewPrivate(qMRMLTreeView& object)
  : q_ptr(&object)
{
  this->SceneModel = 0;
  this->SortFilterModel = 0;
  this->FitSizeToVisibleIndexes = true;
  this->TreeViewSizeHint = QSize();
  this->TreeViewMinSizeHint = QSize(120, 120);
  this->ShowScene = true;
  this->ShowRootNode = false;
  this->NodeMenu = 0;
  this->RenameAction = 0;
  this->DeleteAction = 0;
  this->EditAction = 0;
  this->SceneMenu = 0;
}
//------------------------------------------------------------------------------
qMRMLTreeViewPrivate::~qMRMLTreeViewPrivate()
{
}

//------------------------------------------------------------------------------
void qMRMLTreeViewPrivate::init()
{
  Q_Q(qMRMLTreeView);

  q->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
  q->setItemDelegate(new qMRMLItemDelegate(q));
  q->setAutoScrollMargin(32); // scroll hot area sensitivity
  this->setSortFilterProxyModel(new qMRMLSortFilterProxyModel(q));
  q->setSceneModelType("Transform");
  
  //ctkModelTester * tester = new ctkModelTester(p);
  //tester->setModel(this->SortFilterModel);
  //QObject::connect(q, SIGNAL(activated(QModelIndex)),
  //                 q, SLOT(onActivated(QModelIndex)));
  //QObject::connect(q, SIGNAL(clicked(QModelIndex)),
  //                 q, SLOT(onActivated(QModelIndex)));

  q->setUniformRowHeights(true);
  
  QObject::connect(q, SIGNAL(collapsed(QModelIndex)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));
  QObject::connect(q, SIGNAL(expanded(QModelIndex)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));
//QObject::connect(q->header(), SIGNAL(sectionResized(int,int,int)),
  //                  q, SLOT(onSectionResized()));
  q->horizontalScrollBar()->installEventFilter(q);
  
  this->NodeMenu = new QMenu(q);
  this->NodeMenu->setObjectName("nodeMenuTreeView");

  // rename node
  this->RenameAction =
    new QAction(qMRMLTreeView::tr("Rename"),this->NodeMenu);
  this->NodeMenu->addAction(this->RenameAction);
  QObject::connect(this->RenameAction, SIGNAL(triggered()),
                   q, SLOT(renameCurrentNode()));

  // delete node
  this->DeleteAction =
    new QAction(qMRMLTreeView::tr("Delete"),this->NodeMenu);
  this->NodeMenu->addAction(this->DeleteAction);
  QObject::connect(this->DeleteAction, SIGNAL(triggered()),
                   q, SLOT(deleteCurrentNode()));
  // EditAction is hidden by default
  this->EditAction =
    new QAction(qMRMLTreeView::tr("Edit properties..."), this->NodeMenu);
  QObject::connect(this->EditAction, SIGNAL(triggered()),
                   q, SLOT(editCurrentNode()));
  this->SceneMenu = new QMenu(q);
  this->SceneMenu->setObjectName("sceneMenuTreeView");
}

//------------------------------------------------------------------------------
void qMRMLTreeViewPrivate::setSceneModel(qMRMLSceneModel* newModel)
{
  Q_Q(qMRMLTreeView);
  if (!newModel)
    {
    return;
    }

  newModel->setMRMLScene(q->mrmlScene());

  this->SceneModel = newModel;
  this->SortFilterModel->setSourceModel(this->SceneModel);
  q->expandToDepth(2);
}

//------------------------------------------------------------------------------
void qMRMLTreeViewPrivate::setSortFilterProxyModel(qMRMLSortFilterProxyModel* newSortModel)
{
  Q_Q(qMRMLTreeView);
  if (newSortModel == this->SortFilterModel)
    {
    return;
    }
  
  // delete the previous filter
  delete this->SortFilterModel;
  this->SortFilterModel = newSortModel;
  // Set the input of the view
  // if no filter is given then let's show the scene model directly
  q->QTreeView::setModel(this->SortFilterModel
    ? static_cast<QAbstractItemModel*>(this->SortFilterModel)
    : static_cast<QAbstractItemModel*>(this->SceneModel));
  // Setting a new model to the view resets the selection model
  QObject::connect(q->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                   q, SLOT(onCurrentRowChanged(QModelIndex)));
  if (!this->SortFilterModel)
    {
    return;
    }
  this->SortFilterModel->setParent(q);
  // Set the input of the filter
  this->SortFilterModel->setSourceModel(this->SceneModel);

  // resize the view if new rows are added/removed
  QObject::connect(this->SortFilterModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));
  QObject::connect(this->SortFilterModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));

  q->expandToDepth(2);
  q->onNumberOfVisibleIndexChanged();
}

//------------------------------------------------------------------------------
void qMRMLTreeViewPrivate::recomputeSizeHint(bool force)
{
  Q_Q(qMRMLTreeView);
  this->TreeViewSizeHint = QSize();
  if ((this->FitSizeToVisibleIndexes || force) && q->isVisible())
    {
    // TODO: if the number of items changes often, don't update geometry,
    // it might be too expensive, maybe use a timer
    q->updateGeometry();
    }
}

//------------------------------------------------------------------------------
QSize qMRMLTreeViewPrivate::sizeHint()const
{
  Q_Q(const qMRMLTreeView);
  if (!this->FitSizeToVisibleIndexes)
    {
    return q->QTreeView::sizeHint();
    }
  if (this->TreeViewSizeHint.isValid())
    {
    return this->TreeViewSizeHint;
    }
  int visibleIndexCount = 0;
  for(QModelIndex index = this->SortFilterModel->mrmlSceneIndex();
      index.isValid();
      index = q->indexBelow(index))
    {
    ++visibleIndexCount;
    }
  this->TreeViewSizeHint = q->QTreeView::sizeHint();
  this->TreeViewSizeHint.setHeight(
    q->frameWidth()
    + (q->isHeaderHidden() ? 0 : q->header()->sizeHint().height())
    + visibleIndexCount * q->sizeHintForRow(0)
    + (q->horizontalScrollBar()->isVisibleTo(const_cast<qMRMLTreeView*>(q)) ? q->horizontalScrollBar()->height() : 0)
    + q->frameWidth());
  // Add half a line to give some space under the tree
  this->TreeViewSizeHint.rheight() += q->sizeHintForRow(0) / 2;
  this->TreeViewSizeHint =
    this->TreeViewSizeHint.expandedTo(this->TreeViewMinSizeHint);
  return this->TreeViewSizeHint;
}

//------------------------------------------------------------------------------
// qMRMLTreeView
//------------------------------------------------------------------------------
qMRMLTreeView::qMRMLTreeView(QWidget *_parent)
  :QTreeView(_parent)
  , d_ptr(new qMRMLTreeViewPrivate(*this))
{
  Q_D(qMRMLTreeView);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLTreeView::~qMRMLTreeView()
{
}

//------------------------------------------------------------------------------
void qMRMLTreeView::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLTreeView);
  Q_ASSERT(d->SortFilterModel);
  vtkMRMLNode* rootNode = this->rootNode();
  // only qMRMLSceneModel needs the scene, the other proxies don't care.
  d->SceneModel->setMRMLScene(scene);
  this->setRootNode(rootNode);
  this->expandToDepth(2);
}

//------------------------------------------------------------------------------
QString qMRMLTreeView::sceneModelType()const
{
  Q_D(const qMRMLTreeView);
  return d->SceneModelType;
}

//------------------------------------------------------------------------------
void qMRMLTreeView::setSceneModelType(const QString& modelName)
{
  Q_D(qMRMLTreeView);

  qMRMLSceneModel* newModel = 0;
  qMRMLSortFilterProxyModel* newFilterModel = d->SortFilterModel;
  // switch on the incoming model name
  if (modelName == QString("Transform"))
    {
    newModel = new qMRMLSceneTransformModel(this);
    }
  else if (modelName == QString("Displayable"))
    {
    newModel = new qMRMLSceneDisplayableModel(this);
    }
  else if (modelName == QString("ModelHierarchy"))
    {
    newModel = new qMRMLSceneModelHierarchyModel(this);
    newFilterModel = new qMRMLSortFilterHierarchyProxyModel(this);
    }
  else if (modelName == QString(""))
    {
    newModel = new qMRMLSceneModel(this);
    }
  if (newModel)
    {
    d->SceneModelType = modelName;
    newModel->setListenNodeModifiedEvent(this->listenNodeModifiedEvent());
    }
  if (newFilterModel)
    {
    newFilterModel->setNodeTypes(this->nodeTypes());
    newFilterModel->setShowHidden(this->showHidden());
    }
  d->setSceneModel(newModel);
  // typically a no op except for ModelHierarchy
  d->setSortFilterProxyModel(newFilterModel);
}

//------------------------------------------------------------------------------
void qMRMLTreeView::setSceneModel(qMRMLSceneModel* newSceneModel, const QString& modelType)
{
  Q_D(qMRMLTreeView);

  if (!newSceneModel) 
    {
    return;
    }
  d->SceneModelType = modelType;
  d->setSceneModel(newSceneModel);
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLTreeView::mrmlScene()const
{
  Q_D(const qMRMLTreeView);
  return d->SceneModel ? d->SceneModel->mrmlScene() : 0;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLTreeView::currentNode()const
{
  Q_D(const qMRMLTreeView);
  return d->SortFilterModel->mrmlNodeFromIndex(this->selectionModel()->currentIndex());
}

//------------------------------------------------------------------------------
void qMRMLTreeView::setCurrentNode(vtkMRMLNode* node)
{
  Q_D(const qMRMLTreeView);
  QModelIndex nodeIndex = d->SortFilterModel->indexFromMRMLNode(node);
  this->setCurrentIndex(nodeIndex);
}

//------------------------------------------------------------------------------
void qMRMLTreeView::onCurrentRowChanged(const QModelIndex& index)
{
  Q_D(qMRMLTreeView);
  Q_ASSERT(d->SortFilterModel);
  Q_ASSERT(this->currentNode() == d->SortFilterModel->mrmlNodeFromIndex(index));
  emit currentNodeChanged(d->SortFilterModel->mrmlNodeFromIndex(index));
}

//------------------------------------------------------------------------------
void qMRMLTreeView::setListenNodeModifiedEvent(bool listen)
{
  Q_D(qMRMLTreeView);
  Q_ASSERT(d->SceneModel);
  d->SceneModel->setListenNodeModifiedEvent(listen);
}

//------------------------------------------------------------------------------
bool qMRMLTreeView::listenNodeModifiedEvent()const
{
  Q_D(const qMRMLTreeView);
  return d->SceneModel ? d->SceneModel->listenNodeModifiedEvent() : true;
}

// --------------------------------------------------------------------------
QStringList qMRMLTreeView::nodeTypes()const
{
  return this->sortFilterProxyModel()->nodeTypes();
}

// --------------------------------------------------------------------------
void qMRMLTreeView::setNodeTypes(const QStringList& _nodeTypes)
{
  this->sortFilterProxyModel()->setNodeTypes(_nodeTypes);
}

//--------------------------------------------------------------------------
bool qMRMLTreeView::isRenameMenuActionVisible()const
{
  Q_D(const qMRMLTreeView);
  return d->NodeMenu->actions().contains(d->RenameAction);
}

//--------------------------------------------------------------------------
void qMRMLTreeView::setRenameMenuActionVisible(bool show)
{
  Q_D(qMRMLTreeView);
  if (show)
    {
    // Prepend the action in the menu
    this->prependNodeMenuAction(d->RenameAction);
    }
  else
    {
    d->NodeMenu->removeAction(d->RenameAction);
    }
}

//--------------------------------------------------------------------------
bool qMRMLTreeView::isDeleteMenuActionVisible()const
{
  Q_D(const qMRMLTreeView);
  return d->NodeMenu->actions().contains(d->DeleteAction);
}

//--------------------------------------------------------------------------
void qMRMLTreeView::setDeleteMenuActionVisible(bool show)
{
  Q_D(qMRMLTreeView);
  if (show)
    {
    // Prepend the action in the menu
    this->prependNodeMenuAction(d->DeleteAction);
    }
  else
    {
    d->NodeMenu->removeAction(d->DeleteAction);
    }
}

//--------------------------------------------------------------------------
bool qMRMLTreeView::isEditMenuActionVisible()const
{
  Q_D(const qMRMLTreeView);
  return d->NodeMenu->actions().contains(d->EditAction);
}

//--------------------------------------------------------------------------
void qMRMLTreeView::setEditMenuActionVisible(bool show)
{
  Q_D(qMRMLTreeView);
  if (show)
    {
    // Prepend the action in the menu
    this->prependNodeMenuAction(d->EditAction);
    }
  else
    {
    d->NodeMenu->removeAction(d->EditAction);
    }
}

//--------------------------------------------------------------------------
void qMRMLTreeView::prependNodeMenuAction(QAction* action)
{
  Q_D(qMRMLTreeView);
  // Prepend the action in the menu
  d->NodeMenu->insertAction(d->NodeMenu->actions()[0], action);
}

//--------------------------------------------------------------------------
void qMRMLTreeView::appendNodeMenuAction(QAction* action)
{
  Q_D(qMRMLTreeView);
  // Append the new action to the menu
  d->NodeMenu->addAction(action);
}

//--------------------------------------------------------------------------
void qMRMLTreeView::appendSceneMenuAction(QAction* action)
{
  Q_D(qMRMLTreeView);
  // Appends the new action to the menu
  d->SceneMenu->addAction(action);
}

//--------------------------------------------------------------------------
void qMRMLTreeView::prependSceneMenuAction(QAction* action)
{
  Q_D(qMRMLTreeView);
  // Prepend the action in the menu
  QAction* beforeAction =
    d->SceneMenu->actions().size() ? d->SceneMenu->actions()[0] : 0;
  d->SceneMenu->insertAction(beforeAction, action);
}

//--------------------------------------------------------------------------
void qMRMLTreeView::removeNodeMenuAction(QAction* action)
{
  Q_D(qMRMLTreeView);
  d->NodeMenu->removeAction(action);
}

//--------------------------------------------------------------------------
void qMRMLTreeView::editCurrentNode()
{
  if (!this->currentNode())
    {
    // not sure if it's a request to have a valid node.
    Q_ASSERT(this->currentNode());
    return;
    }
  emit editNodeRequested(this->currentNode());
}

//--------------------------------------------------------------------------
void qMRMLTreeView::setShowScene(bool show)
{
  Q_D(qMRMLTreeView);
  if (d->ShowScene == show)
    {
    return;
    }
  vtkMRMLNode* oldRootNode = this->rootNode();
  d->ShowScene = show;
  this->setRootNode(oldRootNode);
}

//--------------------------------------------------------------------------
bool qMRMLTreeView::showScene()const
{
  Q_D(const qMRMLTreeView);
  return d->ShowScene;
}

//--------------------------------------------------------------------------
void qMRMLTreeView::setShowRootNode(bool show)
{
  Q_D(qMRMLTreeView);
  if (d->ShowRootNode == show)
    {
    return;
    }
  vtkMRMLNode* oldRootNode = this->rootNode();
  d->ShowRootNode = show;
  this->setRootNode(oldRootNode);
}

//--------------------------------------------------------------------------
bool qMRMLTreeView::showRootNode()const
{
  Q_D(const qMRMLTreeView);
  return d->ShowRootNode;
}

//--------------------------------------------------------------------------
void qMRMLTreeView::setRootNode(vtkMRMLNode* rootNode)
{
  Q_D(qMRMLTreeView);
  // Need to reset the filter to be able to find indexes from nodes that
  // could potentially be filtered out.
  this->sortFilterProxyModel()->setHideNodesUnaffiliatedWithNodeID(QString());
  QModelIndex treeRootIndex;
  if (rootNode == 0)
    {
    if (!d->ShowScene)
      {
      treeRootIndex = this->sortFilterProxyModel()->mrmlSceneIndex();
      }
    }
  else
    {
    treeRootIndex = this->sortFilterProxyModel()->indexFromMRMLNode(rootNode);
    if (d->ShowRootNode)
      {
      // Hide the siblings of the root node.
      this->sortFilterProxyModel()->setHideNodesUnaffiliatedWithNodeID(
        rootNode->GetID());
      // The parent of the root node becomes the root for QTreeView.
      treeRootIndex = treeRootIndex.parent();
      rootNode = this->sortFilterProxyModel()->mrmlNodeFromIndex(treeRootIndex);
      }
    }
  qvtkReconnect(this->rootNode(), rootNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateRootNode(vtkObject*)));
  qDebug() << rootNode << treeRootIndex;
  this->setRootIndex(treeRootIndex);
}

//--------------------------------------------------------------------------
vtkMRMLNode* qMRMLTreeView::rootNode()const
{
  Q_D(const qMRMLTreeView);
  vtkMRMLNode* treeRootNode =
    this->sortFilterProxyModel()->mrmlNodeFromIndex(this->rootIndex());
  if (d->ShowRootNode &&
      this->mrmlScene() &&
      this->sortFilterProxyModel()->hideNodesUnaffiliatedWithNodeID()
        .isEmpty())
    {
    return this->mrmlScene()->GetNodeByID(
      this->sortFilterProxyModel()->hideNodesUnaffiliatedWithNodeID().toLatin1());
    }
  return treeRootNode;
}

//--------------------------------------------------------------------------
void qMRMLTreeView::updateRootNode(vtkObject* node)
{
  // Maybe the node has changed of QModelIndex, need to resync
  this->setRootNode(vtkMRMLNode::SafeDownCast(node));
}

//--------------------------------------------------------------------------
qMRMLSortFilterProxyModel* qMRMLTreeView::sortFilterProxyModel()const
{
  Q_D(const qMRMLTreeView);
  Q_ASSERT(d->SortFilterModel);
  return d->SortFilterModel;
}

//--------------------------------------------------------------------------
qMRMLSceneModel* qMRMLTreeView::sceneModel()const
{
  Q_D(const qMRMLTreeView);
  Q_ASSERT(d->SceneModel);
  return d->SceneModel;
}

//--------------------------------------------------------------------------
QSize qMRMLTreeView::minimumSizeHint()const
{
  Q_D(const qMRMLTreeView);
  return d->sizeHint();
}

//--------------------------------------------------------------------------
QSize qMRMLTreeView::sizeHint()const
{
  Q_D(const qMRMLTreeView);
  return d->sizeHint();
}

//--------------------------------------------------------------------------
void qMRMLTreeView::updateGeometries()
{
  // don't update the geometries if it's not visible on screen
  // UpdateGeometries is for tree child widgets geometry
  if (!this->isVisible())
    {
    return;
    }
  this->QTreeView::updateGeometries();
}

//--------------------------------------------------------------------------
void qMRMLTreeView::onNumberOfVisibleIndexChanged()
{
  Q_D(qMRMLTreeView);
  d->recomputeSizeHint();
}

//--------------------------------------------------------------------------
void qMRMLTreeView::setFitSizeToVisibleIndexes(bool enable)
{
  Q_D(qMRMLTreeView);
  d->FitSizeToVisibleIndexes = enable;
  d->recomputeSizeHint(true);
}

//--------------------------------------------------------------------------
bool qMRMLTreeView::fitSizeToVisibleIndexes()const
{
  Q_D(const qMRMLTreeView);
  return d->FitSizeToVisibleIndexes;
}

//--------------------------------------------------------------------------
void qMRMLTreeView::setMinSizeHint(QSize min)
{
  Q_D(qMRMLTreeView);
  d->TreeViewMinSizeHint = min;
  d->recomputeSizeHint();
}

//--------------------------------------------------------------------------
QSize qMRMLTreeView::minSizeHint()const
{
  Q_D(const qMRMLTreeView);
  return d->TreeViewMinSizeHint;
}

//------------------------------------------------------------------------------
void qMRMLTreeView::mousePressEvent(QMouseEvent* e)
{
  Q_D(qMRMLTreeView);
  this->QTreeView::mousePressEvent(e);
  
  if (e->button() != Qt::RightButton)
    {
    return;
    }
  qDebug() << "Mouse press";
  // get the index of the current column
  QModelIndex index = this->indexAt(e->pos());
  
  vtkMRMLNode* node = this->sortFilterProxyModel()->mrmlNodeFromIndex(index);
  
  if (node)
    {
    d->NodeMenu->exec(e->globalPos());
    }
  else if (index == this->sortFilterProxyModel()->mrmlSceneIndex())
    {
    d->SceneMenu->exec(e->globalPos());
    }
}

//------------------------------------------------------------------------------
void qMRMLTreeView::mouseReleaseEvent(QMouseEvent* e)
{
  if (e->button() == Qt::LeftButton)
    {
    // get the index of the current column
    QModelIndex index = this->indexAt(e->pos());
    QStyleOptionViewItemV4 opt = this->viewOptions();
    opt.rect = this->visualRect(index);
    qobject_cast<qMRMLItemDelegate*>(this->itemDelegate())->initStyleOption(&opt,index);
    QRect decorationElement =
      this->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, this);
    //decorationElement.translate(this->visualRect(index).topLeft());
    if (decorationElement.contains(e->pos()))  
      {
      if (this->clickDecoration(index))
        {
        return;
        }
      }
    }

  this->QTreeView::mouseReleaseEvent(e);
}

//------------------------------------------------------------------------------
bool qMRMLTreeView::clickDecoration(const QModelIndex& index)
{
  bool res = false;
  QModelIndex sourceIndex = this->sortFilterProxyModel()->mapToSource(index);
  if (!(sourceIndex.flags() & Qt::ItemIsEnabled))
    {
    res = false;
    }
  else if (sourceIndex.column() == this->sceneModel()->visibilityColumn())
    {
    this->toggleVisibility(index);
    res = true;
    }

  if (res)
    {
    emit decorationClicked(index);
    }
  return res;
}

//------------------------------------------------------------------------------
void qMRMLTreeView::toggleVisibility(const QModelIndex& index)
{
  vtkMRMLNode* node = this->sortFilterProxyModel()->mrmlNodeFromIndex(index);
  vtkMRMLDisplayNode* displayNode =
    vtkMRMLDisplayNode::SafeDownCast(node);
  vtkMRMLDisplayableNode* displayableNode =
    vtkMRMLDisplayableNode::SafeDownCast(node);
  vtkMRMLDisplayableHierarchyNode* displayableHierarchyNode =
      vtkMRMLDisplayableHierarchyNode::SafeDownCast(node);
  if (displayableHierarchyNode)
    {
    vtkMRMLDisplayNode *hierDisplayNode = displayableHierarchyNode->GetDisplayNode();
    int visibility = 1;
    if (hierDisplayNode)
      {
      visibility = (hierDisplayNode->GetVisibility() ? 0 : 1);
      }
    this->mrmlScene()->StartState(vtkMRMLScene::BatchProcessState);
    vtkMRMLModelHierarchyLogic::SetChildrenVisibility(displayableHierarchyNode,visibility);
    this->mrmlScene()->EndState(vtkMRMLScene::BatchProcessState);
    }
  else if (displayNode)
    {
    displayNode->SetVisibility(displayNode->GetVisibility() ? 0 : 1);
    }
  else if (displayableNode)
    {
    displayableNode->SetDisplayVisibility(displayableNode->GetDisplayVisibility() ? 0 : 1);
    }
}

//------------------------------------------------------------------------------
void qMRMLTreeView::renameCurrentNode()
{
  if (!this->currentNode())
    {
    Q_ASSERT(this->currentNode());
    return;
    }
  // pop up an entry box for the new name, with the old name as default
  QString oldName = this->currentNode()->GetName();

  bool ok = false;
  QString newName = QInputDialog::getText(
    this, "Rename " + oldName, "New name:",
    QLineEdit::Normal, oldName, &ok);
  if (!ok)
    {
    return;
    }
  this->currentNode()->SetName(newName.toLatin1());
  emit currentNodeRenamed(newName);
}

//------------------------------------------------------------------------------
void qMRMLTreeView::deleteCurrentNode()
{
//  Q_D(qMRMLTreeView);

  if (!this->currentNode())
    {
    Q_ASSERT(this->currentNode());
    return;
    }
  this->mrmlScene()->RemoveNode(this->currentNode());
  emit currentNodeDeleted(this->currentIndex());
}

//------------------------------------------------------------------------------
bool qMRMLTreeView::isAncestor(const QModelIndex& index, const QModelIndex& potentialAncestor)
{
  QModelIndex ancestor = index.parent();
  while(ancestor.isValid())
    {
    if (ancestor == potentialAncestor)
      {
      return true;
      }
    ancestor = ancestor.parent();
    }
  return false;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLTreeView::findAncestor(const QModelIndex& index, const QModelIndexList& potentialAncestors)
{
  foreach(const QModelIndex& potentialAncestor, potentialAncestors)
    {
    if (qMRMLTreeView::isAncestor(index, potentialAncestor))
      {
      return potentialAncestor;
      }
    }
  return QModelIndex();
}

//------------------------------------------------------------------------------
QModelIndexList qMRMLTreeView::removeChildren(const QModelIndexList& indexes)
{
  QModelIndexList noAncestorIndexList;
  foreach(QModelIndex index, indexes)
    {
    if (!qMRMLTreeView::findAncestor(index, indexes).isValid())
      {
      noAncestorIndexList << index;
      }
    }
  return noAncestorIndexList;
}

//------------------------------------------------------------------------------
void qMRMLTreeView::showEvent(QShowEvent* event)
{
  Q_D(qMRMLTreeView);
  this->Superclass::showEvent(event);
  if (d->FitSizeToVisibleIndexes &&
      !d->TreeViewSizeHint.isValid())
    {
    this->updateGeometry();
    }
}

//------------------------------------------------------------------------------
bool qMRMLTreeView::eventFilter(QObject* object, QEvent* e)
{
  Q_D(qMRMLTreeView);
  bool res = this->QTreeView::eventFilter(object, e);
  // When the horizontal scroll bar is shown/hidden, the sizehint should be
  // updated ?
  if (d->FitSizeToVisibleIndexes &&
      object == this->horizontalScrollBar() &&
      (e->type() == QEvent::Show ||
       e->type() == QEvent::Hide))
    {
    d->recomputeSizeHint();
    }
  return res;
}
