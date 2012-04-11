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

#ifndef DICOMTAGTYPE_H
#define DICOMTAGTYPE_H

#include <string>
#include <vector>
#include <boost/serialization/access.hpp>
#include <boost/shared_ptr.hpp>

/*!
	A structure to represent the dicom tags.
*/
struct DicomTagType {
  std::string name;
  std::string tag;
  DicomTagType(const std::string &n, const std::string &t):name(n), tag(t) {}
  private:
    DicomTagType(){}
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};
typedef std::vector< DicomTagType > DicomTagList;
typedef boost::shared_ptr< DicomTagList > DicomTagListPointer;

#endif // DICOMTAGTYPE_H
