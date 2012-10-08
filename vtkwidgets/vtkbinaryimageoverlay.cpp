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

#include "vtkbinaryimageoverlay.h"
#include <vtkMatrix4x4.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkScalarsToColors.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include "vtkbinaryimagetocolor.h"
#include "vtkinteractorstyleprojectionview.h"
#include <boost/bind.hpp>


vtkBinaryImageOverlay::vtkBinaryImageOverlay( vtkRenderer *renderer,
			  vtkInteractorStyleProjectionView *interactorStyle,
			  const ActionDispatch &action, vtkImageData *image, vtkMatrix4x4 *reslicePlaneTransform,
			  const RGBType &color, int &actionHandle, double opacity)
  :m_image(image),
  m_reslice(vtkImageReslice::New()),
  m_colormap(vtkImageMapToColors::New()),
  m_lookup(vtkBinaryImageToColor::New()),
  m_actor(vtkImageActor::New()),
  m_renderer( renderer ),
  m_interactorStyle(interactorStyle), actionHandle(-1) {
    
  m_lookup->SetColor(color);
    
  m_actor->SetOpacity(opacity);
  
  m_reslice->SetOutputDimensionality(2);
  m_reslice->SetBackgroundLevel(-1000);
  
  m_colormap->SetLookupTable(m_lookup);
  m_colormap->SetInputConnection(m_reslice->GetOutputPort());

  m_actor->SetInput(m_colormap->GetOutput());
  m_reslice->SetResliceAxes(reslicePlaneTransform);
  m_image->UpdateInformation();
  m_reslice->SetInput( m_image );

  m_reslice->SetOutputSpacing(1,1,1);
  
  if (m_renderer)
    m_renderer->AddActor(m_actor);
  
  if (action.valid) {
    action.sig->connect( boost::bind(&vtkImageReslice::Modified, m_reslice) );
    this->actionHandle = interactorStyle->addAction(action);
  }
  actionHandle = this->actionHandle;
}

vtkBinaryImageOverlay::~vtkBinaryImageOverlay() {
  if (m_renderer) {
    m_renderer->RemoveActor( m_actor );
  }
  if (actionHandle!=-1 && m_interactorStyle) 
    m_interactorStyle->removeAction( actionHandle );
  if (m_reslice) m_reslice->Delete();
  if (m_colormap) m_colormap->Delete();
  if (m_lookup) m_lookup->Delete();
  if (m_actor) m_actor->Delete();
}

void vtkBinaryImageOverlay::resize( unsigned int xres, unsigned int yres ) {
  m_reslice->SetOutputExtent(0,xres,0,yres,0,0);
  m_reslice->SetOutputOrigin(xres/-2.0,yres/-2.0,0);
}

void vtkBinaryImageOverlay::activateAction() {
  m_interactorStyle->activateAction(actionHandle);
}

bool vtkBinaryImageOverlay::operator<(const vtkBinaryImageOverlay &other) const {
  return this->m_image < other.m_image;
}
