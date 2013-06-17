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

#include "vtkbinaryimagetocolor.h"
#include <vtkObjectFactory.h>
#include "imagedefinitions.h"

vtkStandardNewMacro(vtkBinaryImageToColor);
vtkCxxRevisionMacro(vtkBinaryImageToColor, "$Revision: 0.1 $");

vtkBinaryImageToColor::vtkBinaryImageToColor()
  :m_upperColor(m_colorColor), m_lowerColor(m_blackColor), 
  m_dUpperColor(m_dColorColor), m_dLowerColor(m_dBlackColor) {
  SetRange(BinaryPixelOff, BinaryPixelOn);
  RGBType black;
  black[0] = 0;black[1] = 0;black[2] = 0;
  SetColor(black);
  m_blackColor[0] = 0;m_blackColor[1] = 0;m_blackColor[2] = 0;m_blackColor[3] = 0;
  m_dBlackColor[0] = 0;m_dBlackColor[1] = 0;m_dBlackColor[2] = 0;
}

vtkBinaryImageToColor::vtkBinaryImageToColor(const RGBType &color)
  :m_upperColor(m_colorColor), m_lowerColor(m_blackColor), 
  m_dUpperColor(m_dColorColor), m_dLowerColor(m_dBlackColor) {
  SetRange(BinaryPixelOff, BinaryPixelOn);
  SetColor(color);
  m_blackColor[0] = 0;m_blackColor[1] = 0;m_blackColor[2] = 0;m_blackColor[3] = 0;
  m_dBlackColor[0] = 0;m_dBlackColor[1] = 0;m_dBlackColor[2] = 0;
}

double *vtkBinaryImageToColor::GetRange() {
  return m_range.c_array();
}

void vtkBinaryImageToColor::SetRange(double min, double max) {
  m_range[0] = min;
  m_range[1] = max;
  m_dthreshold = (min+max)*.5;
  m_threshold = static_cast<unsigned char>(m_dthreshold+.5);
  adjustUpperLower();
}

unsigned char* vtkBinaryImageToColor::MapValue(double value) {
  if (value>=m_threshold) return m_upperColor.c_array();
  else return m_lowerColor.c_array();
}

void vtkBinaryImageToColor::GetColor(double value, double rgb[3]) {
  if (value>=m_threshold) {
    memcpy(rgb, m_dUpperColor.c_array(), sizeof(double) * 3);
  } else {
    memcpy(rgb, m_dLowerColor.c_array(), sizeof(double) * 3);
  }
}

void vtkBinaryImageToColor::MapScalarsThroughTable2(void *input, unsigned char *output,
    int inputDataType, int numberOfValues, int numberOfComponents, int outputFormat) {
  if (numberOfComponents!=1 || inputDataType != VTK_UNSIGNED_CHAR) return;
  unsigned char *inputData = static_cast<unsigned char*>(input);
  unsigned char *sourceColor;
  if (outputFormat == VTK_RGBA) {
    for(int i=0;i<numberOfValues;i++) {
      sourceColor = m_lowerColor.c_array();
      if ((*inputData) >= m_threshold) {
	sourceColor = m_upperColor.c_array();
      }
      memcpy(output, sourceColor, sizeof(unsigned char) * 4);
      output+=4;
      inputData++;
    }
  } else if (outputFormat == VTK_RGB) {
    for(int i=0;i<numberOfValues;i++) {
      sourceColor = m_lowerColor.c_array();
      if ((*inputData) >= m_threshold) {
	sourceColor = m_upperColor.c_array();
      }
      memcpy(output, sourceColor, sizeof(unsigned char) * 3);
      output+=3;
      inputData++;
    }
  }
}

void vtkBinaryImageToColor::SetColor(const RGBType &c) {
  m_upperColor[0] = c[0];
  m_upperColor[1] = c[1];
  m_upperColor[2] = c[2];
  m_upperColor[3] = 255;
  m_dUpperColor[0] = c[0]/255.0;m_dUpperColor[1] = c[1]/255.0;m_dUpperColor[2] = c[2]/255.0;
  adjustUpperLower();
}

void vtkBinaryImageToColor::adjustUpperLower(void) {
  m_upperColor = m_colorColor; m_dUpperColor = m_dColorColor;
  m_lowerColor = m_blackColor; m_dLowerColor = m_dBlackColor;
  if (m_range[0] > m_range[1]) {
    std::swap(m_upperColor, m_lowerColor);
    std::swap(m_dUpperColor, m_dLowerColor);
  }
}
