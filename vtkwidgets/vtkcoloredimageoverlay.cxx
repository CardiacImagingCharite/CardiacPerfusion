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


vtkColoredImageOverlay::vtkColoredImageOverlay( vtkRenderer *renderer,
			  vtkInteractorStyleProjectionView *interactorStyle,
			  const ActionDispatch &action, vtkImageData *image, vtkMatrix4x4 *reslicePlaneTransform,
			  int &actionHandle, double opacity)
  :m_image(image),
  m_reslice(vtkImageReslice::New()),
  m_colorMapper(vtkImageMapToColors::New()),
  m_colorMap(vtkLookupTable::New()),
  m_actor(vtkImageActor::New()),
  m_renderer( renderer ),
  m_interactorStyle(interactorStyle), actionHandle(-1) {
    
	//Set the shape of the colorMap to linear
	  m_colorMap->SetRampToLinear();
	//Set number of created colors
	m_colorMap->SetNumberOfTableValues(256);
	//Set the range of the image values
	m_colorMap->SetTableRange( 0, 10);
	//Set the range of the available colors
	m_colorMap->SetHueRange(0.0,0.667);
	//Set the saturation range of the colors
	//m_colorMap->SetSaturationRange( 0.7, 1 );
	//set the brightness of the colors
	m_colorMap->SetValueRange( 0.8, 1 );
	//generate LUT
	m_colorMap->Build();
	//in order to hide the maximum and minimum values, 
	//set the alpha of the borders to zero
	
	//create alpha ramp at the borders 
	for(int i = 0; i < 16; i++)
	{
		double rgba[4];
		m_colorMap->GetTableValue(i,rgba);
		m_colorMap->SetTableValue(i,rgba[0],rgba[1],rgba[2],1/16*i);

		m_colorMap->GetTableValue(255-i,rgba);
		m_colorMap->SetTableValue(255-i,rgba[0],rgba[1],rgba[2],1/16*i);
	}

//	m_colorMap->SetTableValue(99,0,0,0,0);

	m_colorMap->SetAlpha(opacity);

	double f[2];
	m_colorMap->GetAlphaRange(f);

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

	if (action.valid) {
		action.sig->connect( boost::bind(&vtkImageReslice::Modified, m_reslice) );
		this->actionHandle = interactorStyle->addAction(action);
	}
	actionHandle = this->actionHandle;
}

vtkColoredImageOverlay::~vtkColoredImageOverlay() {
  if (m_renderer) {
    m_renderer->RemoveActor( m_actor );
  }
  if (actionHandle!=-1 && m_interactorStyle) 
    m_interactorStyle->removeAction( actionHandle );
  if (m_reslice) m_reslice->Delete();
  if (m_colorMap) m_colorMap->Delete();
  //if (m_colorMapper) m_colorMapper->Delete();
  if (m_actor) m_actor->Delete();
}

void vtkColoredImageOverlay::resize( unsigned int xres, unsigned int yres ) {

	m_reslice->SetOutputExtent(0,xres,0,yres,0,0);
	m_reslice->SetOutputOrigin(xres/-2.0,yres/-2.0,0);
}

void vtkColoredImageOverlay::activateAction() {
  m_interactorStyle->activateAction(actionHandle);
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
		