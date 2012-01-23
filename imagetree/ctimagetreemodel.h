/*
    This file is part of perfusionkit.
    Copyright 2010 Henning Meyer

    perfusionkit is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    perfusionkit is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with perfusionkit.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CTIMAGETREEMODEL_H
#define CTIMAGETREEMODEL_H

#include <QAbstractItemModel>
#include <string>
#include <vector>
#include <boost/serialization/access.hpp>
#include <itkMetaDataDictionary.h>
#include "vtkconnectordatabase.h"
#include "dicomtagtype.h"


class CTImageTreeModel : public QAbstractItemModel {
  Q_OBJECT
  public:
    CTImageTreeModel(const DicomTagList &header, QObject *parent = 0);
    ~CTImageTreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder );

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    
    void appendFilename( const itk::MetaDataDictionary &dict, const std::string &fname);
    void insertItemCopy(const TreeItem& item);
    void loadAllImages(void);
    const TreeItem &getItem(const QModelIndex &index) const;
    TreeItem &getItem(const QModelIndex &index);
    
    void openModelFromFile(const std::string &fname);
    void saveModelToFile(const std::string &fname);
    
    size_t getMaxImageMemoryUsage() const { return maxImageMemoryUsage; }
    void setMaxImageMemoryUsage(size_t s);
    void initMaxMemoryUsage();
    void registerConnectorData(VTKConnectorDataBasePtr p);
    
    void setSerializationPath( const std::string p ) { serializationPath = p; }
    const std::string &getSerializationPath() const { return serializationPath; }
  
    friend class TreeItem;
    friend class CTImageTreeItem;
    
    
  public slots:
    bool removeItem(const QModelIndex &idx);
    
  private:
    friend class boost::serialization::access;
    
    static const QString MaxImageMemoryUsageSettingName;
    
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
    
    CTImageTreeModel(QObject *parent = 0):QAbstractItemModel(parent),rootItem(this) {
      initMaxMemoryUsage();
    }
    void emitLayoutAboutToBeChanged() { emit layoutAboutToBeChanged(); }
    void emitLayoutChanged() { emit layoutChanged(); }
    QModelIndex createIndex(int r, int c, const TreeItem*p) const;
    DicomTagListPointer HeaderFields;
    TreeItem rootItem;
    size_t maxImageMemoryUsage;
    typedef std::list<VTKConnectorDataBasePtr> ConnectorDataStorageType;
    ConnectorDataStorageType ConnectorDataStorage;
    std::string serializationPath;
};


#endif // CTIMAGETREEMODEL_H
