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

#ifndef VTKCONNECTORDATABASE_H
#define VTKCONNECTORDATABASE_H

#include <boost/shared_ptr.hpp>
#include "treeitem.h"

class vtkImageData;

class VTKConnectorDataBase {
  public:
  virtual vtkImageData *getVTKImageData() const = 0;
  virtual size_t getSize() const = 0;
  const TreeItem *getBaseItem() const { return baseItem; }
  TreeItem *getBaseItem() { return baseItem; }
  VTKConnectorDataBase(
    TreeItem *baseItem_): baseItem(baseItem_) {}
  private:
  TreeItem *baseItem;
};

typedef boost::shared_ptr< VTKConnectorDataBase > VTKConnectorDataBasePtr;
    
    
#endif // VTKCONNECTORDATABASE_H
