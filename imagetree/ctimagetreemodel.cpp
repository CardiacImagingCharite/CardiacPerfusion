/*
    This file is part of KardioPerfusion.
    Copyright 2012 Christian Freye

	This file was part of perfusionkit (Copyright 2010 Henning Meyer)
	and was modified and extended to fit the actual needs. 

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
*/

#include "ctimagetreemodel.h"
#include <QProgressDialog>
#include <QApplication>
#include <QSettings>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
//#include "ctimagetreemodel_serializer.h"
#include "ctimagetreeitem.h"


const QString CTImageTreeModel::MaxImageMemoryUsageSettingName("MaxImageMemoryUsage");


CTImageTreeModel::CTImageTreeModel(const DicomTagList &header, QObject *parent)
  : QAbstractItemModel(parent), rootItem( this ) {
    HeaderFields = boost::make_shared<DicomTagList>(header);
    initMaxMemoryUsage();
}

void CTImageTreeModel::initMaxMemoryUsage() {
    QSettings settings;
    unsigned defaultMaxUsage = 2048UL * 1024UL * 1024UL;
    maxImageMemoryUsage = settings.value(MaxImageMemoryUsageSettingName, defaultMaxUsage ).toULongLong();
}


CTImageTreeModel::~CTImageTreeModel() {
}

bool CTImageTreeModel::hasChildren ( const QModelIndex & parent) const {
  return (getItem(parent).childCount() > 0);
  
}

void CTImageTreeModel::setMaxImageMemoryUsage(size_t s) { 
  maxImageMemoryUsage = s;
  QSettings settings;
  settings.setValue(MaxImageMemoryUsageSettingName, static_cast<unsigned long long>(maxImageMemoryUsage));
}

void CTImageTreeModel::registerConnectorData(VTKConnectorDataBasePtr p) {
  ConnectorDataStorageType::iterator it = std::find(ConnectorDataStorage.begin(), ConnectorDataStorage.end(), p);
  if (it != ConnectorDataStorage.end()) ConnectorDataStorage.erase(it);
  ConnectorDataStorage.push_back(p);
  size_t sum = 0;
  BOOST_FOREACH(VTKConnectorDataBasePtr &ptr, ConnectorDataStorage) {
    sum += ptr->getSize();
  }
  bool deleted = true;
  while (deleted && sum > maxImageMemoryUsage) {
    deleted = false;
    ConnectorDataStorageType::iterator it = ConnectorDataStorage.begin();
    while(it != ConnectorDataStorage.end()) {
      if (it->unique()) {
	sum -= (*it)->getSize();
	ConnectorDataStorage.erase(it);
	deleted = true;
	break;
      }
      ++it;
    }
  }
}


QVariant CTImageTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole) {
    if ( section < 0 || section >= int(HeaderFields->size()) ) return QVariant::Invalid;
	return QString::fromAscii((*HeaderFields)[section].name.c_str());
  } else {
    return QVariant::Invalid;
  }  
}

void CTImageTreeModel::sort(int column, Qt::SortOrder order) {
  rootItem.sortChildren(column, order == Qt::AscendingOrder);
}


QModelIndex CTImageTreeModel::index(int row, int column, const QModelIndex &parent) const {
  const TreeItem &pItem = getItem(parent);
  if (pItem.childCount() == 0) return QModelIndex();
  if (row >= int(pItem.childCount())) row = pItem.childCount()-1;
  const TreeItem &childItem = pItem.child(row);
  return createIndex(row, column, &childItem);
}

QModelIndex CTImageTreeModel::parent(const QModelIndex &index) const {
  if (!index.isValid())
      return QModelIndex();

  const TreeItem &childItem = getItem(index);
  const TreeItem *parentItem = childItem.parent();

  if (parentItem == &rootItem || parentItem == NULL)
      return QModelIndex();

  return createIndex(parentItem->childNumber(), 0, parentItem);
}

int CTImageTreeModel::rowCount(const QModelIndex &parent) const {
  return getItem(parent).childCount();  
}

int CTImageTreeModel::columnCount(const QModelIndex &parent) const {
  return HeaderFields->size();
}

Qt::ItemFlags CTImageTreeModel::flags(const QModelIndex &index) const {
  return getItem(index).flags(index.column());
}

QVariant CTImageTreeModel::data(const QModelIndex &index, int role) const {
  return getItem(index).data(index.column(), role);
}

bool CTImageTreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (role == Qt::EditRole) return getItem(index).setData(index.column(), value);
  else return false;
}

TreeItem &CTImageTreeModel::getItem(const QModelIndex &index) {
  if (index.isValid()) {
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if (item) return *item;
  }
  return rootItem;
}

const TreeItem &CTImageTreeModel::getItem(const QModelIndex &index) const {
  if (index.isValid()) {
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if (item) return *item;
  }
  return rootItem;
}

TreeItem &CTImageTreeModel::getRootItem() {
  return rootItem;
}

const TreeItem &CTImageTreeModel::getRootItem() const {
  return rootItem;
}

void CTImageTreeModel::appendFilename( const itk::MetaDataDictionary &dict, const std::string &fname) {
  std::string iUID;
  CTImageTreeItem::getUIDFromDict( dict, iUID );
  if (iUID.empty()) return;
  bool found = false;
  CTImageTreeItem *c;
  int cc = rootItem.childCount();
  for(int i=0; i < cc; i++) {
    c = dynamic_cast<CTImageTreeItem*>(&rootItem.child(i));
    if (c->getUID() == iUID) {
      found = true;
      break;
    }
  }
  if (!found) {
    c = new CTImageTreeItem( &rootItem, HeaderFields, dict );
    rootItem.insertChild(c);
  }
  c->appendFileName(fname);
}

void CTImageTreeModel::insertItemCopy(const TreeItem& item) {
  rootItem.insertChild(item.clone(&rootItem));
}

bool CTImageTreeModel::removeItem(const QModelIndex &idx) {
  TreeItem &item = getItem(idx);
  if (&item != &rootItem) {
    TreeItem *parentItem = item.parent();
    if (parentItem) {
      bool result = parentItem->removeChildren(item.childNumber());
      return result;
    }
  }
  return false;
}

void CTImageTreeModel::loadAllImages(void) {
  const int progressScale = 10000;
  QProgressDialog progressDialog(tr("Loading Volumes..."), tr("Abort"), 0, progressScale);
  progressDialog.setMinimumDuration(1000);
  progressDialog.setWindowModality(Qt::WindowModal);
  const int scalePerVolume = progressScale/rootItem.childCount();
  for(unsigned int i=0; i < rootItem.childCount(); i++) {
    dynamic_cast<CTImageTreeItem&>(rootItem.child(i)).getVTKConnector(&progressDialog, scalePerVolume, scalePerVolume*i );
    if (progressDialog.wasCanceled()) break;
  }
}

 
QModelIndex CTImageTreeModel::createIndex(int r, int c, const TreeItem*p) const {
  if (p == NULL || p == &rootItem) return QModelIndex();
  return QAbstractItemModel::createIndex(r, c, const_cast<TreeItem*>(p));
}


/*void CTImageTreeModel::openModelFromFile(const std::string &fname) {
  deserializeCTImageTreeModelFromFile( *this, fname );
}

void CTImageTreeModel::saveModelToFile(const std::string &fname) {
  serializeCTImageTreeModelToFile(*this, fname);
}
*/

