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

class TreeItem : boost::noncopyable {
  public:
    struct TreeTrouble : public std::exception { };
    TreeItem(TreeItem *parent);
    explicit TreeItem(CTImageTreeModel *model);
    ~TreeItem();
    TreeItem &child(unsigned int number);
    const TreeItem &child(unsigned int number) const;
    unsigned int childCount() const;
    void cloneChildren(TreeItem *dest) const;
    QVariant data(int column, int role = Qt::DisplayRole) const;
    
    virtual TreeItem *clone(TreeItem *clonesParent) const;
    virtual int columnCount() const;
    
    virtual QVariant do_getData_DisplayRole(int column) const { return QVariant::Invalid; }
    virtual QVariant do_getData_UserRole(int column) const { return do_getData_DisplayRole(column); }
    virtual QVariant do_getData_ForegroundRole(int column) const { return QVariant::Invalid; }
    virtual QVariant do_getData_BackgroundRole(int column) const { return QVariant::Invalid; }
    virtual QVariant do_getData_FontRole(int column) const;
    
    virtual Qt::ItemFlags flags(int column) const;
    virtual const std::string &getUID(void) const { static std::string t; return t; }
    
    unsigned int depth(void) const;
    void sortChildren( int column, bool ascending=true );
    bool insertChild(TreeItem *child, unsigned int position);
    bool insertChild(TreeItem *child);
    bool claimChild(TreeItem *child);
    TreeItem *parent();
    const TreeItem *parent() const;
    bool removeChildren(unsigned int position, unsigned int count=1);
    int childNumber() const;
    virtual bool setData(int column, const QVariant &value);
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
