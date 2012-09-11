/*
    Copyright 2012 Christian Freye

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

#ifndef VTKCOLOREDIMAGEOVERLAY_H
#define VTKCOLOREDIMAGEOVERLAY_H

#include "vtkinteractoractiondispatch.h"
#include "imagedefinitions.h"
#include <vtkSmartPointer.h>

class vtkImageData;
class vtkImageReslice;
class vtkImageMapToColors;
class vtkColorMap;
class vtkLookupTable;
class vtkImageActor;
class vtkMatrix4x4;
class vtkInteractorStyleProjectionView;
class vtkRenderer;
class vtkScalarBarActor;

/*! \class vtkColoredImageOverlay vtkColoredImageOverlay.h "vtkColoredImageOverlay.h"
 *  \brief That class contains a colored overlay image.
 */
class vtkColoredImageOverlay {
  public:
	///Constructor
    vtkColoredImageOverlay( vtkRenderer *renderer,
			   vtkInteractorStyleProjectionView *interactorStyle,
			  const ActionDispatch &action, vtkImageData *image, vtkMatrix4x4 *reslicePlaneTransform,
			  int &actionHandle, vtkLookupTable* customColorMap);
    ///Destructor
	~vtkColoredImageOverlay();

	///Gets the image data,
	/*!
	\return Returns the overlay image.
	*/
    vtkImageData *getImage() const { return m_image; }
	///Activates the overlay image.
    void activateAction();
	///Gets the ColorMap
	/*!
	\return Return the color map.
	*/
	vtkLookupTable* getColorMap() {return m_colorMap; }
	///Sets the ColorMap
	/*!
	\param The color map.
	*/
	void setColorMap(vtkLookupTable* cm);
		

	///Gets the actual actor.
	/*!
	\return Return the image actor.
	*/
    vtkImageActor *getActor() { return m_actor; }
	
    bool operator<(const vtkColoredImageOverlay &other) const;
	///Resizes the image.
	/*!
	\param x New target size in x direction.
	\param y New target size in y direction.
	*/
    void resize( unsigned int x, unsigned int y );
	///Hide the legend.
	void hideLegend();
	///Show the legend.
	void showLegend();

  protected:
  vtkImageData *m_image; ///< volume image data to be displayed
  vtkImageReslice *m_reslice; ///< vtkImageAlgorithm to reslice the image
  vtkImageMapToColors *m_colorMapper; ///< used to apply Color
  vtkLookupTable *m_colorMap;		///< color LookUpTable
  vtkSmartPointer<vtkScalarBarActor> m_legend;
//  vtkBinaryImageToColor *m_lookup; ///< color lookup
  vtkImageActor *m_actor; ///< vtkActor which actually displays the resliced volume
  vtkRenderer *m_renderer;
  vtkInteractorStyleProjectionView *m_interactorStyle;
  int actionHandle;
};
#endif