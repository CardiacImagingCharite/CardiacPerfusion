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

#ifndef IMAGEDEFINITIONS_H
#define IMAGEDEFINITIONS_H

#include <itkImage.h>
#include <boost/array.hpp>

const unsigned int ImageDimension = 3;

typedef float RealPixelType;
typedef itk::Image< RealPixelType, ImageDimension> RealImageType;///< for perfusion map images

typedef unsigned char LabelPixelType;
typedef itk::Image< LabelPixelType, ImageDimension> LabelImageType;
//typedef bool BinaryPixelType;
typedef unsigned char BinaryPixelType;
const BinaryPixelType BinaryPixelOff = 0;
const BinaryPixelType BinaryPixelOn = 255;
typedef itk::Image< BinaryPixelType, ImageDimension> BinaryImageType;

typedef signed short CTPixelType;

typedef itk::Image< CTPixelType, ImageDimension> CTImageType;///< for original dicom CT images

typedef boost::array< unsigned char, 3 > RGBType;


#endif // IMAGEDEFINITIONS_H
