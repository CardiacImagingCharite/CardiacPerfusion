/*
    This file is part of KardioPerfusion.
    Copyright 2012 Christian Freye

	This file was part of perfusionkit (Copyright 2010 Henning Meyer)
	and was modified and extended to fit the actual needs. 

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
*/

#ifndef VTKBINARYIMAGETOCOLOR_H
#define VTKBINARYIMAGETOCOLOR_H
#include <vtkScalarsToColors.h>
#include "imagedefinitions.h"
#include <boost/array.hpp>


/*! \class vtkBinaryImageToColor vtkBinaryImageToColor.h "vtkBinaryImageToColor.h"
 *  \brief 
 */
class vtkBinaryImageToColor: public vtkScalarsToColors {
  public:
  vtkTypeRevisionMacro(vtkBinaryImageToColor,vtkScalarsToColors);
  static vtkBinaryImageToColor *New();
  vtkBinaryImageToColor();
  vtkBinaryImageToColor(const RGBType &color);
  virtual double *GetRange();
  virtual void SetRange(double min, double max);
  virtual unsigned char* MapValue(double value);
  virtual void GetColor(double value, double rgb[3]);
  virtual void MapScalarsThroughTable2(void *input, unsigned char *output,
    int inputDataType, int numberOfValues, int numberOfComponents, int outputFormat);
  void SetColor(const RGBType &color);
  void SetRandomColor();
  private:
	  vtkIdType GetNumberOfAvailableColors(){return 0;}
    void adjustUpperLower(void);
    boost::array< double, 2> range;
    unsigned char threshold;
    double dthreshold;
    boost::array< unsigned char, 4> &upperColor, &lowerColor, blackColor, colorColor;
    boost::array< double,3 > &dUpperColor, &dLowerColor, dBlackColor, dColorColor;
};



#endif // VTKBINARYIMAGETOCOLOR_H
