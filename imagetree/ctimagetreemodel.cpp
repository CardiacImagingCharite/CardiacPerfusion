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

#include "ctimagetreemodel.h"
#include <QProgressDialog>
#include <QApplication>
#include <QSettings>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include "ctimagetreemodel_serializer.h"
#include "ctimagetreeitem.h"
#include "itkShrinkAverageFilter.h"


const QString CTImageTreeModel::MaxImageMemoryUsageSettingName("MaxImageMemoryUsage");


CTImageTreeModel::CTImageTreeModel(const DicomTagList &header, QObject *parent)
  : QAbstractItemModel(parent), m_rootItem( this ) {
    m_HeaderFields = boost::make_shared<DicomTagList>(header);
    initMaxMemoryUsage();
}

void CTImageTreeModel::initMaxMemoryUsage() {
    QSettings settings;
    unsigned defaultMaxUsage = 10*1024UL * 1024UL * 1024UL;
    m_maxImageMemoryUsage = settings.value(MaxImageMemoryUsageSettingName, defaultMaxUsage ).toULongLong();
}


CTImageTreeModel::~CTImageTreeModel() {
}

bool CTImageTreeModel::hasChildren ( const QModelIndex & parent) const {
  return (getItem(parent).childCount() > 0);
  
}

void CTImageTreeModel::setMaxImageMemoryUsage(size_t s) { 
  m_maxImageMemoryUsage = s;
  QSettings settings;
  settings.setValue(MaxImageMemoryUsageSettingName, static_cast<unsigned long long>(m_maxImageMemoryUsage));
}

void CTImageTreeModel::registerConnectorData(VTKConnectorDataBasePtr p) {
  ConnectorDataStorageType::iterator it = std::find(m_ConnectorDataStorage.begin(), m_ConnectorDataStorage.end(), p);
  if (it != m_ConnectorDataStorage.end()) m_ConnectorDataStorage.erase(it);
  m_ConnectorDataStorage.push_back(p);
  size_t sum = 0;
  BOOST_FOREACH(VTKConnectorDataBasePtr &ptr, m_ConnectorDataStorage) {
    sum += ptr->getSize();
  }
  bool deleted = true;
  while (deleted && sum > m_maxImageMemoryUsage) {
    deleted = false;
    ConnectorDataStorageType::iterator it = m_ConnectorDataStorage.begin();
    while(it != m_ConnectorDataStorage.end()) {
      if (it->unique()) {
	sum -= (*it)->getSize();
	m_ConnectorDataStorage.erase(it);
	deleted = true;
	break;
      }
      ++it;
    }
  }
}


QVariant CTImageTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole) {
    if ( section < 0 || section >= int(m_HeaderFields->size()) ) return QVariant::Invalid;
	return QString::fromAscii((*m_HeaderFields)[section].name.c_str());
  } else {
    return QVariant::Invalid;
  }  
}

void CTImageTreeModel::sort(int column, Qt::SortOrder order) {
  m_rootItem.sortChildren(column, order == Qt::AscendingOrder);
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

  if (parentItem == &m_rootItem || parentItem == NULL)
      return QModelIndex();

  return createIndex(parentItem->childNumber(), 0, parentItem);
}

int CTImageTreeModel::rowCount(const QModelIndex &parent) const {
  return getItem(parent).childCount();  
}

int CTImageTreeModel::columnCount(const QModelIndex &parent) const {
  return m_HeaderFields->size();
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
  return m_rootItem;
}

const TreeItem &CTImageTreeModel::getItem(const QModelIndex &index) const {
  if (index.isValid()) {
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if (item) return *item;
  }
  return m_rootItem;
}

TreeItem &CTImageTreeModel::getRootItem() {
  return m_rootItem;
}

const TreeItem &CTImageTreeModel::getRootItem() const {
  return m_rootItem;
}

void CTImageTreeModel::appendFilename( const itk::MetaDataDictionary &dict, const std::string &fname) {
  std::string iUID;
  CTImageTreeItem::getUIDFromDict( dict, iUID );
  if (iUID.empty()) return;
  bool found = false;
  CTImageTreeItem *c;
  int cc = m_rootItem.childCount();
  for(int i=0; i < cc; i++) {
    c = dynamic_cast<CTImageTreeItem*>(&m_rootItem.child(i));
    if (c->getUID() == iUID) {
      found = true;
      break;
    }
  }
  if (!found) {
    c = new CTImageTreeItem( &m_rootItem, m_HeaderFields, dict );
    m_rootItem.insertChild(c);
  }
  c->appendFileName(fname);
}

void CTImageTreeModel::insertItemCopy(const TreeItem& item) {
  m_rootItem.insertChild(item.clone(&m_rootItem));
}

void CTImageTreeModel::removeAllItems() 
{
	for (int i = this->rowCount() -1; i >= 0; i--)
	{
		QModelIndex ImIdx = this->index(i, 1);
		this->removeItem( ImIdx );
	}
}

bool CTImageTreeModel::removeItem(const QModelIndex &idx) {
  TreeItem &item = getItem(idx);
  if (&item != &m_rootItem) {
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
  const int scalePerVolume = progressScale/m_rootItem.childCount();
  for(unsigned int i=0; i < m_rootItem.childCount(); i++) {
    dynamic_cast<CTImageTreeItem&>(m_rootItem.child(i)).getVTKConnector(&progressDialog, scalePerVolume, scalePerVolume*i );
    if (progressDialog.wasCanceled()) break;
  }
}

 
QModelIndex CTImageTreeModel::createIndex(int r, int c, const TreeItem*p) const {
  if (p == NULL || p == &m_rootItem) return QModelIndex();
  return QAbstractItemModel::createIndex(r, c, const_cast<TreeItem*>(p));
}


void CTImageTreeModel::openModelFromFile(const std::string &fname) {
  deserializeCTImageTreeModelFromFile( *this, fname );
}

void CTImageTreeModel::saveModelToFile(const std::string &fname) {
	shrinkAllCTImageTreeItems();
	serializeCTImageTreeModelToFile(*this, fname);
}

void CTImageTreeModel::shrinkAllCTImageTreeItems()
{
	typedef itk::ShrinkAverageFilter<CTImageType, CTImageType> ShrinkAverageFilterType;
	
	int rows = this->rowCount();
	
	for ( int i = 0; i < rows; i++ )
	{
		QModelIndex ImIdx = this->index(i, 1);
		TreeItem* item = &this->getItem(ImIdx);
		ITKVTKTreeItem<CTImageType> *currentItem = dynamic_cast<ITKVTKTreeItem<CTImageType>*>(item);
		typename CTImageType::Pointer ImagePtr = currentItem->getITKImage();

		typename ShrinkAverageFilterType::Pointer shrinkAverageFilter = ShrinkAverageFilterType::New();
		shrinkAverageFilter->SetInput( ImagePtr );
		shrinkAverageFilter->SetShrinkFactor(0, 2);
		shrinkAverageFilter->SetShrinkFactor(1, 2);
		shrinkAverageFilter->SetShrinkFactor(2, 2);
		shrinkAverageFilter->Update();
		typename CTImageType::Pointer outImage = shrinkAverageFilter->GetOutput();

		currentItem->setITKImage(outImage);
	}
}


