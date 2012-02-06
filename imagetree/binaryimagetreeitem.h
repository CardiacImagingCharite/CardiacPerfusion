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

#ifndef BINARYIMAGETREEITEM_H
#define BINARYIMAGETREEITEM_H

#include "itkvtktreeitem.h"
#include "imagedefinitions.h"
#include <QString>
#include <QColor>
#include <boost/function.hpp>
#include <boost/serialization/access.hpp>

class BinaryImageTreeItem : public ITKVTKTreeItem< BinaryImageType > {
  public:
    typedef ITKVTKTreeItem< BinaryImageType > BaseClass;
    BinaryImageTreeItem(TreeItem * parent, ImageType::Pointer itkImage, const QString &name);
    virtual TreeItem *clone(TreeItem *clonesParent=NULL) const;
    virtual int columnCount() const;
    virtual QVariant do_getData_DisplayRole(int c) const;
    virtual QVariant do_getData_BackgroundRole(int column) const;
    virtual Qt::ItemFlags flags(int column) const;
    virtual bool setData(int c, const QVariant &value);
    const QString &getName() const { return name; }
    void setName(const QString &_name) { name = _name; }
    void drawSphere( float radius, float x, float y, float z, bool erase );
    void regionGrow( float x, float y, float z, int threshold, boost::function<void()> postGrowingAction);
    const QColor &getColor() const { return color;}
    void setColor(const QColor &newColor) { color = newColor; }
    void thresholdParent(double lower, double upper);
    bool watershedParent(const BinaryImageTreeItem *includedSegment, const BinaryImageTreeItem *excludedSegment);
    void binaryDilate(int iterations);
    void binaryErode(int iterations);
    double getVolumeInML(void) const;
    virtual bool isA(const std::type_info &other) const { 
      if (typeid(BinaryImageTreeItem)==other) return true;
      if (typeid(BaseClass)==other) return true;
      if (typeid(BaseClass::BaseClass)==other) return true;
      return false;
    }
    
  private:
    void createRandomColor();
    QString name;
    QColor color;
    
  protected:    
    BinaryImageTreeItem():volumeMtime(0) {};

  private:
    long unsigned volumeMtime;
    double volumeInML;
    ConnectorHandle imageKeeper;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};




#endif // BINARYIMAGETREEITEM_H
