/*
    Copyright 2012 Charit� Universit�tsmedizin Berlin, Institut f�r Radiologie
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
    boost::array< double, 2> m_range;
    unsigned char m_threshold;
    double m_dthreshold;
    boost::array< unsigned char, 4> &m_upperColor, &m_lowerColor, m_blackColor, m_colorColor;
    boost::array< double,3 > &m_dUpperColor, &m_dLowerColor, m_dBlackColor, m_dColorColor;
};



#endif // VTKBINARYIMAGETOCOLOR_H
