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
