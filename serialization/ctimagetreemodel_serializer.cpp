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

#include "ctimagetreemodel_serializer.h"

#include <fstream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>

#include "ctimagetreemodel.h"
#include "ctimagetreeitem.h"
#include "binaryimagetreeitem.h"
#include "realimagetreeitem.h"
#include "serialization_helper.h"

//#include <boost/serialization/export.hpp>
//BOOST_CLASS_EXPORT_GUID(CTImageTreeItem, "CTImageTreeItem");
//BOOST_CLASS_EXPORT_GUID(BinaryImageTreeItem, "BinaryImageTreeItem");





void deserializeCTImageTreeModelFromFile(CTImageTreeModel &model, const std::string &fname) {
  using namespace std;
  using namespace boost::iostreams;
  std::ifstream inFileStream( fname.c_str(), ios_base::in | ios_base::binary  );
  filtering_stream<input> inStreamFilter;
  inStreamFilter.push(zlib_decompressor());
  inStreamFilter.push(inFileStream);
  boost::archive::binary_iarchive ia( inStreamFilter );
  model.setSerializationPath( fname );
  ia >> model;
}

void serializeCTImageTreeModelToFile(CTImageTreeModel &model, const std::string &fname) {
  using namespace std;
  using namespace boost::iostreams;
  ofstream outFileStream( fname.c_str(), ios_base::out | ios_base::binary );
  filtering_stream<output> outStreamFilter;
  zlib_params zp(zlib::best_speed);
  outStreamFilter.push(zlib_compressor(zp));
  outStreamFilter.push(outFileStream);
  boost::archive::binary_oarchive oa( outStreamFilter );
  model.setSerializationPath( fname );
  oa << model;
}

