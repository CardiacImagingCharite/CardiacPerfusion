/*
    Copyright 2012 Charit� Universit�tsmedizin Berlin, Institut f�r Radiologie

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

#include "vtkcoloredimageoverlay.h"
#include <vtkMatrix4x4.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkLookupTable.h>
#include <vtkScalarsToColors.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkScalarBarActor.h>
#include "vtkinteractorstyleprojectionview.h"
#include <boost/bind.hpp>
#include <itkBSplineInterpolateImageFunction.h>
#include "realimagetreeitem.h"

vtkColoredImageOverlay::vtkColoredImageOverlay( vtkRenderer *renderer,
			  vtkInteractorStyleProjectionView *interactorStyle,
			  const ActionDispatch &action, vtkImageData *image, vtkMatrix4x4 *reslicePlaneTransform,
			  int &actionHandle, vtkLookupTable* customColorMap)
  :m_image(image),
  m_reslice(vtkImageReslice::New()),
  m_colorMapper(vtkImageMapToColors::New()),
  m_colorMap(customColorMap),
  m_actor(vtkImageActor::New()),
  m_renderer( renderer ),
  m_interactorStyle(interactorStyle), m_actionHandle(-1) {
    
	

	m_colorMapper->SetInputConnection( m_reslice->GetOutputPort());
	m_colorMapper->SetLookupTable( m_colorMap );
	
	//m_actor->SetOpacity(opacity);
  
	m_reslice->SetOutputDimensionality(2);
	m_reslice->SetBackgroundLevel(-1000);
  
	//m_colormap->SetLookupTable(m_lookup);
	//m_colormap->SetInputConnection(m_reslice->GetOutputPort());

	m_actor->SetInput(m_colorMapper->GetOutput());
	m_reslice->SetResliceAxes(reslicePlaneTransform);
	m_reslice->SetInterpolationModeToCubic();

	m_image->UpdateInformation();
	m_reslice->SetInput( m_image );

	m_reslice->SetOutputSpacing(1,1,1);

	//create legend for LUT
	m_legend = vtkSmartPointer<vtkScalarBarActor>::New();
  
	//create legend for the LUT
	m_legend->SetLookupTable(m_colorMap);
	m_legend->SetTitle("Values");
	m_legend->SetOrientationToHorizontal();
	m_legend->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    m_legend->GetPositionCoordinate()->SetValue(0.57, 0.87);
    m_legend->SetWidth(0.4);
    m_legend->SetHeight(0.15);

	if (m_renderer)
	{
		m_renderer->AddActor(m_actor);
		m_renderer->AddActor(m_legend);
	}

	if (action.m_valid) {
		action.m_sig->connect( boost::bind(&vtkImageReslice::Modified, m_reslice) );
		this->m_actionHandle = interactorStyle->addAction(action);
	}
	actionHandle = this->m_actionHandle;
}

vtkColoredImageOverlay::~vtkColoredImageOverlay() {
  if (m_renderer) {
    m_renderer->RemoveActor( m_actor );
  }
  if (m_actionHandle!=-1 && m_interactorStyle) 
    m_interactorStyle->removeAction( m_actionHandle );
  if (m_reslice) m_reslice->Delete();
  if (m_colorMap) m_colorMap->Delete();
  //if (m_colorMapper) m_colorMapper->Delete();
  if (m_actor) m_actor->Delete();
}

void vtkColoredImageOverlay::resize( unsigned int xres, unsigned int yres ) {

//	typedef itk::BSplineInterpolateImageFunction< RealImageTreeItem::ImageType > InterpolatorType;
//	InterpolatorType::Pointer interpolator = InterpolatorType::New();
	
	m_reslice->SetInterpolationModeToCubic();
	m_reslice->SetOutputExtent(0,xres,0,yres,0,0);
	m_reslice->SetOutputOrigin(xres/-2.0,yres/-2.0,0);
}

void vtkColoredImageOverlay::activateAction() {
  m_interactorStyle->activateAction(m_actionHandle);
}

bool vtkColoredImageOverlay::operator<(const vtkColoredImageOverlay &other) const {
  return this->m_image < other.m_image;
}

void vtkColoredImageOverlay::hideLegend()
{
	this->m_renderer->RemoveActor(m_legend);
}

void vtkColoredImageOverlay::showLegend()
{
	this->m_renderer->AddActor(m_legend);
}

void vtkColoredImageOverlay::setColorMap(vtkLookupTable* cm)
{
	m_colorMap = cm; 
	m_colorMapper->SetLookupTable( m_colorMap );
	m_legend->SetLookupTable(m_colorMap);
}