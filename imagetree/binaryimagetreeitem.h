/*
    Copyright 2012 Christian Freye
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

#ifndef BINARYIMAGETREEITEM_H
#define BINARYIMAGETREEITEM_H

#include "itkvtktreeitem.h"
#include "imagedefinitions.h"
#include <QString>
#include <QColor>
#include <boost/function.hpp>
#include <boost/serialization/access.hpp>

/*! \class BinaryImageTreeItem BinaryImageTreeItem.h "BinaryImageTreeItem.h"
 *  \brief This is the representation of an image overlay which can be placed in a TreeView.
 */
class BinaryImageTreeItem : public ITKVTKTreeItem< BinaryImageType > {
  public:
    typedef ITKVTKTreeItem< BinaryImageType > BaseClass;
	/// Constructor of the class.
	/*!
	\param parent A pointer to the parent element of the segment.
	\param itkImage A pointer to the associated itkImage,
	\param name The name of the segment.
	*/
    BinaryImageTreeItem(TreeItem * parent, ImageType::Pointer itkImage, const QString &name);
	///Clones an existing TreeItem.
	/*!
	\return Pointer to a TreeItem.
	*/
    virtual TreeItem *clone(TreeItem *clonesParent=NULL) const;
	///Get the column count (always 1, because there is only the name of the Item).
    /*!
	\return Number of columns.
	*/
	virtual int columnCount() const;
	///Get Name of the TreeItem.
	/*!
	\return Name of the TreeItem.
	*/
    virtual QVariant do_getData_DisplayRole(int c) const;
	///Getter for the color.
	/*!
	\return The color of the TreeItem.
	*/
    virtual QVariant do_getData_BackgroundRole(int column) const;
	///Getter for the properties.
	/*!
	\return The properties of the TreeItem.
	*/
    virtual Qt::ItemFlags flags(int column) const;
	///Setter for the properties of the TreeItem.
	/*!
	\param c Defines the column
	\param value Contains the value, that is to be set.

	\return True if succeed.
	*/
    virtual bool setData(int c, const QVariant &value);

	///Getter for the item name
	/*!
	\return Name of the TreeItem.
	*/
    const QString &getName() const { return name; }
	///Setter for the item name
	/*!
	\param _name Contains the new name of the TreeItem.
	*/
    void setName(const QString &_name) { name = _name; }

	///Draws or erases a sphere at the overlay 
	/*!
	\param radius The radius of the sphere.
	\param x The point on the x-axis.
	\param y The point on the y-axis.
	\param z The point on the z-axis.
	\param erase Indicates if the sphere is to be erased.
	*/
    void drawSphere( float radius, float x, float y, float z, bool erase );
	///Applies a regionGrow algorithm from a given seed with a specific threshold to the parent image.
	/*!
	\param x The point of the seed on the x-axis.
	\param y The point of the seed on the y-axis.
	\param z The point of the seed on the z-axis.
	\param threshold Contains the threshold for the surrounding of the seed.
	\param postGrowingAction This function is called after the regionGrow algorithm.
	*/
    void regionGrow( float x, float y, float z, int threshold, boost::function<void()> postGrowingAction);
	///Sets the pixel at a given position to 255
	/*!
	\param x The point of the seed on the x-axis.
	\param y The point of the seed on the y-axis.
	\param z The point of the seed on the z-axis.
	*/
	void setPixel(float x, float y, float z);

	///Getter for the item color.
    /*!
	\return The color of the segment.
	*/
	const QColor &getColor() const { return color;}
	///Setter for the item color.
	/*!
	\param newColot Contains the new item color.
	*/
    void setColor(const QColor &newColor) { color = newColor; }
    ///Applies a threshold filter with given borders to the parent image.
	/*!
	\param lower The lower gray value.
	\param upper The upper gray value.
	*/
	void thresholdParent(double lower, double upper);

//    bool watershedParent(const BinaryImageTreeItem *includedSegment, const BinaryImageTreeItem *excludedSegment);
    
	///Dilates the binary overlay image with a given number of iterations.
	/*!
	\param iterations The number of iterations.
	*/
	void binaryDilate(int iterations);
    ///Erodes the binary overlay image with a given number of iteration
	/*!
	\param iterations The number of iterations.
	*/
	void binaryErode(int iterations);

	/// Get the volume in ml.
	/*!
	\return The volume in ml.
	*/
    double getVolumeInML(void) const;

	///Identifies the type of a TreeItem.
	/*!
	\param other Contains the compare type.

	\return Returns TRUE if the type matches otherwise FALSE.
	*/
    virtual bool isA(const std::type_info &other) const { 
      if (typeid(BinaryImageTreeItem)==other) return true;
      if (typeid(BaseClass)==other) return true;
      if (typeid(BaseClass::BaseClass)==other) return true;
      return false;
    }
    
  private:
	///Creates a random color and sets it to the overlay.
    void createRandomColor();
    QString name; // name of the TreeItem
    QColor color;// color of the TreeItem
    
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
