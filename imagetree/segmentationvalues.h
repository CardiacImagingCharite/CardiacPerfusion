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

#ifndef SEGMENTATIONVALUES_H
#define SEGMENTATIONVALUES_H

#include <boost/serialization/access.hpp>
#include "itkvtktreeitem.h"
#include "imagedefinitions.h"



struct SegmentationValues {
  enum Accuracy {
    SimpleAccuracy,
    PreventDoubleSamplingAccuracy,
    InterpolatedAccuray
  };
  const ITKVTKTreeItem<BinaryImageType> *segment;
  long unsigned mtime;
  double mean;
  double stddev;
  int min;
  int max;
  int sampleCount;
  Accuracy accuracy;
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

#endif // SEGMENTATIONVALUES_H
