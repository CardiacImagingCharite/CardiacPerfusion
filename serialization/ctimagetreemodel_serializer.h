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

#ifndef CTIMAGETREEMODEL_SERIALIZER_H
#define CTIMAGETREEMODEL_SERIALIZER_H

#include <string>
#include "ctimagetreemodel.h"

///Deserializes an Image Tree
	/*!
	\param model The resulting model.
	\param fname The filename of the serialized model.
	*/
void deserializeCTImageTreeModelFromFile(CTImageTreeModel &model, const std::string &fname);
///Serializes an Image Tree
	/*!
	\param model The Image Tree Model, which should be serialized.
	\param fname The filename for the model.
	*/
void serializeCTImageTreeModelToFile(CTImageTreeModel &model, const std::string &fname);





#endif //CTIMAGETREEMODEL_SERIALIZER_H