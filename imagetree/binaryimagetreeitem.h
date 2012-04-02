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
	// Constructor, which takes its parent, an Image and a name
    BinaryImageTreeItem(TreeItem * parent, ImageType::Pointer itkImage, const QString &name);
	//clones an existing TreeItem
    virtual TreeItem *clone(TreeItem *clonesParent=NULL) const;
	//returns the number of columns (always 1, cause there is only the name of the Item)
    virtual int columnCount() const;
	//returns the name of the TreeItem
    virtual QVariant do_getData_DisplayRole(int c) const;
	//returns the color of the TreeItem
    virtual QVariant do_getData_BackgroundRole(int column) const;
	//returns the properties of a TreeItem
    virtual Qt::ItemFlags flags(int column) const;
	//sets the name of the TreeItem for column 0 and return true, if succeed
    virtual bool setData(int c, const QVariant &value);

	//Getter and Setter for the item name
    const QString &getName() const { return name; }
    void setName(const QString &_name) { name = _name; }

	//draws a sphere with a given radius at a specific position (or erases it)
    void drawSphere( float radius, float x, float y, float z, bool erase );
	//applies a regionGrow algorithm from a given seed with a specific threshold to the parent image
    void regionGrow( float x, float y, float z, int threshold, boost::function<void()> postGrowingAction);

	//Getter and Setter for the item color
    const QColor &getColor() const { return color;}
    void setColor(const QColor &newColor) { color = newColor; }
    //applies a threshold filter with given borders to the parent image
	void thresholdParent(double lower, double upper);

//    bool watershedParent(const BinaryImageTreeItem *includedSegment, const BinaryImageTreeItem *excludedSegment);
    
	//dilates the binary overlay image with a given number of iteration
	void binaryDilate(int iterations);
    //erodes the binary overlay image with a given number of iteration
	void binaryErode(int iterations);
	//applies a canny edge filter to the parent image
	void extractEdges();
	// ???
    double getVolumeInML(void) const;

	//identifies the type of an TreeItem
    virtual bool isA(const std::type_info &other) const { 
      if (typeid(BinaryImageTreeItem)==other) return true;
      if (typeid(BaseClass)==other) return true;
      if (typeid(BaseClass::BaseClass)==other) return true;
      return false;
    }
    
  private:
    void createRandomColor();//creates and sets the color of the overlay
    QString name; // name of the TreeItem
    QColor color;// color of the TreeItem
    
  protected:    
	// ???
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
