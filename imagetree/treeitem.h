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

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QString>
#include <QList>
#include <QVariant>
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <string>
#include <QAbstractItemModel>
#include <boost/serialization/access.hpp>

class CTImageTreeModel;
class BinaryImageTreeItem;
/*! \class TreeItem TreeItem.h "TreeItem.h"
 *  \brief This class represents a tree item.
 */
class TreeItem : boost::noncopyable {
  public:
	///A structure which contains exceptions.
    struct TreeTrouble : public std::exception { };
	///Constructor
    TreeItem(TreeItem *parent);
    explicit TreeItem(CTImageTreeModel *model);
    ///Destructor
	~TreeItem();
	///Gets the child item at a specific number.
	/*!
	\param number Desired number of the child.

	\return Child tree item.
	*/
    TreeItem &child(unsigned int number);
    const TreeItem &child(unsigned int number) const;
    ///Gets the child count.
	/*!
	\return Child count.
	*/
	unsigned int childCount() const;
	///Clones the actual child elements to a specific tree item
	/*!
	\param dest Destination tree item.
	*/
    void cloneChildren(TreeItem *dest) const;
	///Gets the data at a specific column.
	/*!
	\param column The desired column.
	\param role (optional) The role of the data. Default is DisplayRole.

	\return The data at that column.
	*/
    QVariant data(int column, int role = Qt::DisplayRole) const;
	///Clones the actual object.
	/*!
	\param clonesParent The parent element of the clone.

	\return The cloned tree item. 
	*/
    virtual TreeItem *clone(TreeItem *clonesParent) const;
    ///Gets the number of columns.
	/*!
	\return The column count. 
	*/
	virtual int columnCount() const;
    /*!
		\name Getter
		for different roles of the data.
	*/
	///@{
    virtual QVariant do_getData_DisplayRole(int column) const { return QVariant::Invalid; }
    virtual QVariant do_getData_UserRole(int column) const { return do_getData_DisplayRole(column); }
    virtual QVariant do_getData_ForegroundRole(int column) const { return QVariant::Invalid; }
    virtual QVariant do_getData_BackgroundRole(int column) const { return QVariant::Invalid; }
    virtual QVariant do_getData_FontRole(int column) const;
	///@}
    
	///Gets the flags of the tree item
	/*!
	\param column The desired column.

	\return The flags of the element. 
	*/
    virtual Qt::ItemFlags flags(int column) const;
	///Gets the UID of the tree item.
	/*!
	\return The UID.
	*/
    virtual const std::string &getUID(void) const { static std::string t; return t; }
    
    unsigned int depth(void) const;
	///Sorts the children of the actual element.
	/*!
	\param column The reference column.
	\param ascending (optional) The sorting order. Default is TRUE.
	*/
    void sortChildren( int column, bool ascending=true );
	///Inserts a child at a specific position.
	/*!
	\param child The child which is to be added.
	\param position The target position
	*/
    bool insertChild(TreeItem *child, unsigned int position);
	///Insert child at the end of the list.
	/*!
	\param child The child which is to be added.
	*/
    bool insertChild(TreeItem *child);

    bool claimChild(TreeItem *child);
	///Gets the parent of the actual element.
	/*!
	\return Parent of tree item.
	*/
    TreeItem *parent();
    const TreeItem *parent() const;
	///Removes a given number of children beginning at a defined position
	/*!
	\param position The starting postion
	\param count (optional) The numbers of children to be removed. Default is 1.

	\return FALSE if position and cound exeeds the size.
	*/
    bool removeChildren(unsigned int position, unsigned int count=1);
	///Gets the actual child number.
	/*!
	\return The child number.
	*/
    int childNumber() const;
	///Sets data at a given column. Not implemented.
	/*!
	\param column The destination column
	\param value The data to be added.

	\return Returns FALSE.
	*/
    virtual bool setData(int column, const QVariant &value);
	///Gets the model.
	/*!
	\return Returns the actual model.
	*/
    CTImageTreeModel *getModel(void) { return model; }
    void clearActiveDown(void) const;
    bool isActive(void) const { return active; }
    void setActive(bool act=true) const;
    void toggleActive(void) const { setActive(!active); }
    const BinaryImageTreeItem* userSelectSegment(const QString &dialogTitle, const QString &dialogMessage) const ;
    virtual bool isA(const std::type_info &other) const { return (typeid(TreeItem)==other) ? true : false; }
    
    
  protected:
    QModelIndex getIndex(int column=0) const;
    CTImageTreeModel *model;
    TreeItem():model(NULL),parentItem(NULL),active(false) {};
  private:
    typedef boost::ptr_vector<TreeItem> ChildListType;
    ChildListType childItems;
    TreeItem * parentItem;
    bool active;

  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

#endif // TREEITEM_H
