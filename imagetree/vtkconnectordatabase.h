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

#ifndef VTKCONNECTORDATABASE_H
#define VTKCONNECTORDATABASE_H

#include <boost/shared_ptr.hpp>
#include "treeitem.h"

class vtkImageData;

class VTKConnectorDataBase {
  public:
  virtual vtkImageData *getVTKImageData() const = 0;
  virtual size_t getSize() const = 0;
  const TreeItem *getBaseItem() const { return m_baseItem; }
  TreeItem *getBaseItem() { return m_baseItem; }
  VTKConnectorDataBase(
    TreeItem *baseItem_): m_baseItem(baseItem_) {}
  private:
  TreeItem *m_baseItem;
};

typedef boost::shared_ptr< VTKConnectorDataBase > VTKConnectorDataBasePtr;
    
    
#endif // VTKCONNECTORDATABASE_H
