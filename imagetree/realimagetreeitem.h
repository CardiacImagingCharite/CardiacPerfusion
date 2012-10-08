/*
    Copyright 2012 Charité Universitätsmedizin Berlin, Institut für Radiologie

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

#ifndef REALIMAGETREEITEM_H
#define REALIMAGETREEITEM_H

#include "itkvtktreeitem.h"
#include "imagedefinitions.h"
#include <QString>
#include <QColor>
#include <boost/function.hpp>
#include <boost/serialization/access.hpp>
#include <vtkLookupTable.h>

/*! \class BinaryImageTreeItem BinaryImageTreeItem.h "BinaryImageTreeItem.h"
 *  \brief This is the representation of an image overlay which can be placed in a TreeView.
 */
class RealImageTreeItem : public ITKVTKTreeItem< RealImageType > {
  public:
    typedef ITKVTKTreeItem< RealImageType > BaseClass;
	/// Constructor of the class.
	/*!
	\param parent A pointer to the parent element of the segment.
	\param itkImage A pointer to the associated itkImage,
	\param name The name of the segment.
	*/
    RealImageTreeItem(TreeItem * parent, ImageType::Pointer itkImage, const QString &name, const double opacity);
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
	///Setter for the colormap of the item
	/*!
	\param _name Contains the new colormap of the TreeItem.
	*/
    void setColorMap(vtkLookupTable* _colorMap) { m_colorMap = _colorMap; }
	///Getter for the colormap
	/*!
	\return Colormap of the TreeItem.
	*/
	vtkLookupTable* getColorMap() { return m_colorMap; }
	///Setter for the alpha channel of the colormap
	/*!
	\param alpha Opacity of the colormap
	*/
	void setOpacity(double alpha);

	///Draws or erases a sphere at the overlay 
	/*!
	\param radius The radius of the sphere.
	\param x The point on the x-axis.
	\param y The point on the y-axis.
	\param z The point on the z-axis.
	\param erase Indicates if the sphere is to be erased.
	*/
    virtual bool isA(const std::type_info &other) const { 
      if (typeid(RealImageTreeItem)==other) return true;
      if (typeid(BaseClass)==other) return true;
      if (typeid(BaseClass::BaseClass)==other) return true;
      return false;
    }
    
  private:
    QString name; // name of the TreeItem
	vtkLookupTable* m_colorMap;
    
  protected:

  private:
    ConnectorHandle imageKeeper;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

#endif