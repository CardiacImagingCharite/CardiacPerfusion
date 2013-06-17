/*
    Copyright 2012 Charit� Universit�tsmedizin Berlin, Institut f�r Radiologie
	Copyright 2010 Henning Meyer

	This file is part of KardioPerfusion.

    KardioPerfusion is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    KardioPerfusion is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with KardioPerfusion.  If not, see <http://www.gnu.org/licenses/>.

    Diese Datei ist Teil von KardioPerfusion.

    KardioPerfusion ist Freie Software: Sie k�nnen es unter den Bedingungen
    der GNU General Public License, wie von der Free Software Foundation,
    Version 3 der Lizenz oder (nach Ihrer Option) jeder sp�teren
    ver�ffentlichten Version, weiterverbreiten und/oder modifizieren.

    KardioPerfusion wird in der Hoffnung, dass es n�tzlich sein wird, aber
    OHNE JEDE GEW�HRLEISTUNG, bereitgestellt; sogar ohne die implizite
    Gew�hrleistung der MARKTF�HIGKEIT oder EIGNUNG F�R EINEN BESTIMMTEN ZWECK.
    Siehe die GNU General Public License f�r weitere Details.

    Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
    Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
*/

#include "treeitem.h"
#include "boost/bind.hpp"
#include <iostream>
#include <QApplication>
#include <QFont>
#include <QInputDialog>
#include <typeinfo>
#include <boost/foreach.hpp>
#include "binaryimagetreeitem.h"
#include "ctimagetreemodel.h"

TreeItem::TreeItem(CTImageTreeModel *model):m_model(model), m_parentItem(NULL),m_active(false) {
}

TreeItem::TreeItem(TreeItem * parent):m_model(parent->m_model), m_parentItem(parent),m_active(false) {
}

TreeItem::~TreeItem() {
}

void TreeItem::cloneChildren(TreeItem *dest) const {
  for(ChildListType::const_iterator it=m_childItems.begin(); it!=m_childItems.end(); it++)
    dest->insertChild( it->clone(dest) );
}


TreeItem &TreeItem::child(unsigned int number) {
  if (number >= m_childItems.size()) throw TreeTrouble();
  return m_childItems[number];
}

const TreeItem &TreeItem::child(unsigned int number) const {
  if (number >= m_childItems.size()) throw TreeTrouble();
  return m_childItems[number];
}

TreeItem *TreeItem::clone(TreeItem *clonesParent) const {
  TreeItem *c = new TreeItem( clonesParent );
  cloneChildren(c);
  return c;
}

int TreeItem::columnCount() const {
  return 0;
}

QVariant TreeItem::data(int column, int role) const {
  switch(role) {
    case Qt::DisplayRole: return do_getData_DisplayRole( column );
    case Qt::FontRole: return do_getData_FontRole( column );
    case Qt::ForegroundRole: return do_getData_ForegroundRole( column );
    case Qt::BackgroundRole: return do_getData_BackgroundRole( column );
    case Qt::UserRole: return do_getData_UserRole( column );
  }
  return QVariant::Invalid;
}

QVariant TreeItem::do_getData_FontRole(int column) const {
  if (m_active) {
    QFont f = QApplication::font();
    f.setWeight( QFont::Bold );
    return f;
  }
  return QVariant::Invalid;
}


unsigned int TreeItem::childCount() const {
  return m_childItems.size();
}

class TreeItemCompareFunctor {
  int col;
  bool descending;
  public:
    typedef TreeItem first_argument_type;
    typedef TreeItem second_argument_type;
    typedef bool result_type;
  TreeItemCompareFunctor(int column, bool ascending=true):col(column),descending(!ascending) {};
  bool operator()(const TreeItem &x, const TreeItem &y) const {
    const QVariant qx = x.data(col, Qt::UserRole);
    const QVariant qy = y.data(col, Qt::UserRole);
    bool okx, oky;
    double dx = qx.toDouble(&okx);
    double dy = qy.toDouble(&oky);
    if (okx && oky) {
      if (dx == dy) return false;
      return descending ^ (dx < dy);
    } else if (okx) {
      return descending;
    } else if (oky) {
      return !descending;
    } else {
      if (&x == &y) return false;
      return descending ^ (&x < &y);
    }
  }
};

void TreeItem::sortChildren( int column, bool ascending ) {
  m_model->emitLayoutAboutToBeChanged();
  m_childItems.sort(
    TreeItemCompareFunctor( column, ascending ) );
  m_model->emitLayoutChanged();
}

unsigned int TreeItem::depth(void) const {
  const TreeItem *parent = m_parentItem;
  unsigned int depth = 0;
  while( parent != NULL ) {
    parent = parent->m_parentItem;
    depth++;
  }
  return depth;
}

bool TreeItem::insertChild(TreeItem *child) {
  return insertChild(child, m_childItems.size());
}

class TreeItemEqualFunctor {
    const std::string &uidx;
  public:
    typedef TreeItem first_argument_type;
    typedef TreeItem second_argument_type;
    typedef bool result_type;
  TreeItemEqualFunctor( const TreeItem &item ):uidx( item.getUID() ) {}
  bool operator()(const TreeItem &y) const {
    if (uidx.empty()) return false;
    const std::string uidy = y.getUID();
    if (uidy.empty()) return false;
    return uidx == uidy;
  }
};

bool TreeItem::insertChild(TreeItem *child, unsigned int position) {
  if (position > m_childItems.size())
    return false;
  if (std::find_if(m_childItems.begin(), m_childItems.end(), TreeItemEqualFunctor(*child)) != m_childItems.end())
    return false;

  QModelIndex insertIndex = m_model->createIndex(position,0,this);
  m_model->emitLayoutAboutToBeChanged();
  m_model->beginInsertRows(insertIndex,position,position);
  child->m_parentItem = this;
  child->m_model = this->m_model;
  m_childItems.insert(m_childItems.begin() + position, child);
  m_model->endInsertRows();
  m_model->emitLayoutChanged();
  return true;
}

const TreeItem *TreeItem::parent() const {
  return m_parentItem;
}

TreeItem *TreeItem::parent() {
  return m_parentItem;
}

bool TreeItem::removeChildren(unsigned int position, unsigned int count) {
  if (position + count > m_childItems.size())
    return false;
// TODO: strange: the following does not work - WHY?  
//  model->beginRemoveRows(model->createIndex(0,0,this), position, position+count-1);
  m_model->beginResetModel();
  for (unsigned int row = 0; row < count; ++row)
    m_childItems.release( m_childItems.begin() + position );
//  model->endRemoveRows();
  m_model->endResetModel();
  return true;
}

bool TreeItem::claimChild(TreeItem *child) {
  if (!child) return false;
  int childPos = child->childNumber();
  ChildListType::auto_type childAutoPtr;
  TreeItem *childParent = child->parent();
  m_model->beginResetModel();
  if (childParent) {
    childAutoPtr = childParent->m_childItems.release( childParent->m_childItems.begin() + childPos );
  } else { childAutoPtr.reset( child ); }
  m_childItems.push_back( childAutoPtr.release() );
  m_model->endResetModel();
  return true;
}

int TreeItem::childNumber() const {
  if (m_parentItem != NULL) {
    const ChildListType &pList = m_parentItem->m_childItems;
    for(ChildListType::size_type i = 0; i != pList.size(); ++i)
      if (&pList[i] == this) return i;
  }
  return 0;  
}

bool TreeItem::setData(int column, const QVariant &value) {
  return false;
}

Qt::ItemFlags TreeItem::flags(int column) const {
  return Qt::NoItemFlags;
}

void TreeItem::setActive(bool act) const {
  if (m_active==act) return;
  const_cast<TreeItem*>(this)->m_active = act;
  const_cast<TreeItem*>(this)->m_model->dataChanged(
    getIndex(0),
    getIndex(columnCount()-1));
}

QModelIndex TreeItem::getIndex(int column) const {
  return const_cast<TreeItem*>(this)->m_model->createIndex(childNumber(),column,m_parentItem);
}


void TreeItem::clearActiveDown(void) const {
  setActive(false);
  for(ChildListType::const_iterator i = m_childItems.begin(); i != m_childItems.end(); ++i)
    i->clearActiveDown();
}

const BinaryImageTreeItem* TreeItem::userSelectSegment(const QString &dialogTitle, const QString &dialogMessage) const {
  const BinaryImageTreeItem *selectedSegment = NULL;
  std::list<const BinaryImageTreeItem *> segmentList;
  {
    std::list<const TreeItem *> itemList;
    itemList.push_back( this );
    while(!itemList.empty()) {
      const TreeItem *currentItem = itemList.back();
      itemList.pop_back();
      int cnum = currentItem->childCount();
      for(int i = 0; i < cnum; i++ ) {
	itemList.push_back( &currentItem->child(i) );
      }
      if (typeid(*currentItem) == typeid(BinaryImageTreeItem))
	segmentList.push_back(dynamic_cast<const BinaryImageTreeItem*>(currentItem));
    }
  }
  if (!segmentList.empty()) {
    QStringList nameList;
    BOOST_FOREACH(const BinaryImageTreeItem *seg, segmentList) {
      nameList << seg->getName();
    }
    bool ok;
    QString selectedName = QInputDialog::getItem(NULL, dialogTitle, dialogMessage, nameList, 0, false, &ok);
    BOOST_FOREACH(const BinaryImageTreeItem *seg, segmentList) {
      if (seg->getName() == selectedName) {
	selectedSegment = seg;
	break;
      }
    }
  }
  return selectedSegment;
}

