/*
    Copyright 2012 Charité Universitätsmedizin Berlin, Institut für Radiologie
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

    KardioPerfusion ist Freie Software: Sie können es unter den Bedingungen
    der GNU General Public License, wie von der Free Software Foundation,
    Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren
    veröffentlichten Version, weiterverbreiten und/oder modifizieren.

    KardioPerfusion wird in der Hoffnung, dass es nützlich sein wird, aber
    OHNE JEDE GEWÄHRLEISTUNG, bereitgestellt; sogar ohne die implizite
    Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
    Siehe die GNU General Public License für weitere Details.

    Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
    Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
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

/*! \class CTImageTreeModel CTImageTreeModel.h "CTImageTreeModel.h"
 *  \brief This is the tree model which is the basis of the treeView.
 */
class CTImageTreeModel : public QAbstractItemModel {

  Q_OBJECT
  
	public:
	///Constructor
    CTImageTreeModel(const DicomTagList &header, QObject *parent = 0);
    ///Destructor
	~CTImageTreeModel();

	///Returns the data at a given index.
	/*!
	\param index Defines the position, that is to returned.
	\param role Defines the role of the data.

	\return The data of the tree model.
	*/
    QVariant data(const QModelIndex &index, int role) const;
	///Returns the name of the header.
	/*!
	\param section Defines the needed header field.
	\param role (optional) The role of the data. Default value is DisplayRole.

	\return The name of the headerfield.
	*/
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    ///Sorts the model.
	/*!
	\param column The reference column.
	\param order (optional) The order to sort. Default is ascending order.
	*/
	void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder );
	///Gets the index of the model.
	/*!
	\param row The row of the model.
	\param column The column of the model.
	\param parent (optional) The parent of the model element. Default is a new QModelIndex.

	\return An index of the model.
	*/
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	///Gets the parent of an model index.
	/*!
	\param index The index of the element.

	\return The index of the parent element.
	*/
    QModelIndex parent(const QModelIndex &index) const;
	///Checks if an element has children.
	/*!
	\param parent (optional) The parent element which is to be checked. Default is a new QModelIndex.

	\return TRUE if it has children, else FALSE.
	*/
    bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;
	///Gets the number of rows of the model.
	/*!
	\param parent (optional) The parent element which is to be checked. Default is a new QModelIndex.
	
	\return The number of rows.
	*/
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
	///Gets the number of columns of the model.
	/*!
	\param parent (optional) The parent element which is to be checked. Default is a new QModelIndex.
	
	\return The number of columns.
	*/
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
	///Gets the flags of an model element.
	/*!
	\param index The index of the element.

	\return The flags of the element.
	*/
    Qt::ItemFlags flags(const QModelIndex &index) const;
	///Sets data to a specific element.
	/*!
	\param index The index of the element. 
	\param value The data which is to be set.
	\param role (optional) The role of the data. Default is EditRole.
	*/
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    ///Append a Filename to the model.
	/*!
	\param dict The metadata dictionary of the dicom file.
	\param fname The filename of the dicom file.
	*/
    void appendFilename( const itk::MetaDataDictionary &dict, const std::string &fname);
	///Inserts a copy of a tree item.
	/*!
	\param item The item to insert.
	*/
    void insertItemCopy(const TreeItem& item);
	///Loads all Images.
    void loadAllImages(void);
	///Get a tree item at a specific index.
	/*!
	\param index The desired index.

	\return The tree item at the index.
	*/
    const TreeItem &getItem(const QModelIndex &index) const;
    TreeItem &getItem(const QModelIndex &index);
	///Get the root item of the tree.
	/*!
	\return The roo tree item.
	*/
    const TreeItem &getRootItem() const;
    TreeItem &getRootItem();
    ///Loads an existing model from a file.
	/*!
	\param fname The filename of the model file. 
	*/
    void openModelFromFile(const std::string &fname);
	///Saves the model to a file.
	/*!
	\param fname The filename where the model is to be saved.
	*/
    void saveModelToFile(const std::string &fname);
    ///Gets the size of the maximum memory to be used.
	/*!
	\return Size of the maximum memory.
	*/
    size_t getMaxImageMemoryUsage() const { return m_maxImageMemoryUsage; }
	///Sets the maximum memory.
	/*!
	\param Size of the memory.
	*/
    void setMaxImageMemoryUsage(size_t s);
	///Initialize the memory usage.
    void initMaxMemoryUsage();
    void registerConnectorData(VTKConnectorDataBasePtr p);
    ///Sets the serialization path.
	/*!
	\param p Contains the path name.
	*/
    void setSerializationPath( const std::string p ) { m_serializationPath = p; }
	///Gets the serialization path.
	/*!
	\return Serialization path.
	*/
    const std::string &getSerializationPath() const { return m_serializationPath; }
    ///Removes all items.
    void removeAllItems();
    ///Shrinks all CTImageTreeItem
    void shrinkAllCTImageTreeItems();
  
    friend class TreeItem;
    friend class CTImageTreeItem;
    
    
  public slots:
	///Removes an item at a specific index.
	/*!
	\param idx The index of the element.
	*/
    bool removeItem(const QModelIndex &idx);
    
  private:
    friend class boost::serialization::access;
    
    static const QString MaxImageMemoryUsageSettingName;
    
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
    
    CTImageTreeModel(QObject *parent = 0):QAbstractItemModel(parent),m_rootItem(this) {
      initMaxMemoryUsage();
    }
    void emitLayoutAboutToBeChanged() { emit layoutAboutToBeChanged(); }
    void emitLayoutChanged() { emit layoutChanged(); }
    QModelIndex createIndex(int r, int c, const TreeItem*p) const;
    DicomTagListPointer m_HeaderFields;
    TreeItem m_rootItem;
    size_t m_maxImageMemoryUsage;
    typedef std::list<VTKConnectorDataBasePtr> ConnectorDataStorageType;
    ConnectorDataStorageType m_ConnectorDataStorage;
    std::string m_serializationPath;
};


#endif // CTIMAGETREEMODEL_H
