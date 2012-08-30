/*
    Copyright 2012 Christian Freye

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

#include "realimagetreeitem.h"

// Constructor, which takes its parent, an Image and a name
RealImageTreeItem::RealImageTreeItem(TreeItem * parent, ImageType::Pointer itkImage, const QString &name)
  :BaseClass(parent, itkImage), name(name){
    imageKeeper = getVTKConnector();
}

//clones an existing TreeItem
TreeItem *RealImageTreeItem::clone(TreeItem *clonesParent) const {
	RealImageTreeItem *c = new RealImageTreeItem(clonesParent, peekITKImage(), name );
	cloneChildren(c);
	return c;
}

//returns the number of columns (always 1, cause there is only the name of the Item)
int RealImageTreeItem::columnCount() const {
	return 1;
}

//returns the name of the TreeItem
QVariant RealImageTreeItem::do_getData_DisplayRole(int c) const {
	if (c==0) 
		return name;
	else 
		return QVariant::Invalid;
}

//returns the color of the TreeItem
QVariant RealImageTreeItem::do_getData_BackgroundRole(int column) const {
	return QBrush(QColor(255,0,0));//QBrush( Qt::GlobalColor::red );
}


//returns the properties of a TreeItem
Qt::ItemFlags RealImageTreeItem::flags(int column) const {
    if (column != 0) 
		return Qt::NoItemFlags;
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;  
}

//sets the name of the TreeItem for column 0 and return true, if succeed
bool RealImageTreeItem::setData(int c, const QVariant &value) {
	if (c==0 && static_cast<QMetaType::Type>(value.type()) == QMetaType::QString) {
		name = value.toString();
		return true;
	} 
	return false;
}

template <typename T>
inline T clip(T min, T val, T max) {
	return std::max<T>( std::min<T>( val, max), min );
}

