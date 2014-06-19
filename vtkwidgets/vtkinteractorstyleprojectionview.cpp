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

#include "vtkinteractorstyleprojectionview.h"

#include <vtkObjectFactory.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkImageReslice.h>
#include <vtkImageData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkImageViewer2.h>
#include <vtkLookupTable.h>
#include <vtkCornerAnnotation.h>
#include <vtkPropPicker.h>
#include <vtkSmartPointer.h>
#include <vtkAssemblyPath.h>
#include <vtkPointData.h>
#include <vtkCell.h>
#include <vtkMath.h>
#include <vtkInteractorObserver.h>
#include <vtkRegularPolygonSource.h>
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkCommand.h>
#include <vtkImageReslice.h>
//
#include <vtkPNGWriter.h>
#include <vtkImageShiftScale.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
#include <vtkMetaImageWriter.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkXMLPolyDataWriter.h>
//
#include <string>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>

#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include "vtkcoloredimageoverlay.h"

#include <itkCastImageFilter.h>
#include "ctimagetreeitem.h"
#include "binaryimagetreeitem.h"
#include <itkVTKImageToImageFilter.h>
#include "maxSlopeAnalyzer.h"
#include "ctimagetreemodel.h"
#include <qtreeview.h>
#include <qwt_plot.h>
#include <qwt_legend.h>


#include <boost/assign.hpp>
#include <boost/foreach.hpp>

using namespace std;

const string keySymLeft = "Left";
const string keySymRight = "Right";
const string keySymUp = "Up";
const string keySymDown = "Down";
const string keySymSpace = "space";

vtkStandardNewMacro(vtkInteractorStyleProjectionView);
vtkCxxRevisionMacro(vtkInteractorStyleProjectionView, "$Revision: 0.1 $");

/// default Constructor
vtkInteractorStyleProjectionView::vtkInteractorStyleProjectionView():
  m_ActionNone(-1),
  m_stateRButton(false),
  m_stateLButton(false),
  m_stateMButton(false),
  m_stateCtrl(false),
  m_sliceIncrement(1.0),
  m_leftMBHint( NULL ),
  m_orientation(NULL),
  m_tempTransform( vtkTransform::New() ),
  m_perfusionOverlay(NULL),
  m_circle(vtkRegularPolygonSource::New()),
  m_circleActor(vtkActor::New())
{
	State = VTKIS_NONE;
	UseTimers = 1;
	SetLMBHint(0);
	m_initialState.orientation = vtkMatrix4x4::New();
	resetActions();
	m_tempTransform->PreMultiply();


	vtkSmartPointer<vtkPolyDataMapper> circleMapper
		= vtkSmartPointer<vtkPolyDataMapper>::New();	
	m_circle->GeneratePolygonOff();
	//  m_circle->SetNormal(0,0,1);
	m_circle->SetRadius(2);
	m_circle->Update();
	m_circle->SetNumberOfSides(360);
	
	circleMapper->SetInputConnection(m_circle->GetOutputPort());
	
	m_circleActor->SetMapper(circleMapper);
	m_circleActor->GetProperty()->SetColor(1.0,0.0,0.0);
	m_circleActor->GetProperty()->SetOpacity(0.9);
	m_circleActor->GetProperty()->SetLineWidth(1);
	
//	m_updateEvent = vtkCommand::UserEvent + 1;
}


void vtkInteractorStyleProjectionView::resetActions() {
  m_ActionFirst = m_ActionSlice = addAction("Slice", boost::bind(&vtkInteractorStyleProjectionView::Slice, this, _2), ActionDispatch::MovingAction, ActionDispatch::UnRestricted );
  m_ActionRotate = addAction("Rotate", boost::bind(&vtkInteractorStyleProjectionView::Rotate, this, _1, _2), ActionDispatch::MovingAction, ActionDispatch::Restricted );
  m_ActionSpin = addAction("Spin", boost::bind(&vtkInteractorStyleProjectionView::Spin, this, _1), ActionDispatch::MovingAction, ActionDispatch::UnRestricted );
  m_ActionEmitZoom = addAction("EmitZoom", boost::bind(&vtkInteractorStyleProjectionView::EmitZoom, this, _2 ), ActionDispatch::MovingAction, ActionDispatch::UnRestricted );
  m_ActionPan = addAction("Pan", boost::bind(&vtkInteractorStyleProjectionView::Pan, this, _1, _2), ActionDispatch::MovingAction, ActionDispatch::Restricted );
  m_ActionEmitWindowLevel = addAction("Emit Window/Level", boost::bind(&vtkInteractorStyleProjectionView::EmitWindowLevelDelta, this, _1, _2), ActionDispatch::MovingAction, ActionDispatch::Restricted );
  
  m_ActionWindowLUT = addAction("Window Lookup Table", boost::bind(&vtkInteractorStyleProjectionView::WindowLUTDelta, this, _1, _2), ActionDispatch::MovingAction, ActionDispatch::Restricted );
  
  m_ActionColorPick = addAction("", boost::bind(&vtkInteractorStyleProjectionView::PickPerfusionValues, this), ActionDispatch::ClickingAction, ActionDispatch::Restricted );
  m_ActionShowCircle = addAction("", boost::bind(&vtkInteractorStyleProjectionView::ShowCircle, this), ActionDispatch::MovingAction, ActionDispatch::Restricted );
  
  //m_leftButtonAction = ActionSlice;
  m_leftButtonAction = m_ActionEmitWindowLevel;
  m_interAction = m_ActionNone;
}


int vtkInteractorStyleProjectionView::addAction(const std::string &label, const ActionSignal::slot_type &slot,
						ActionDispatch::ActionType atype, ActionDispatch::RestrictionType restrict) {
  return addAction(ActionDispatch( label, slot, atype, restrict ) );
}
  
int vtkInteractorStyleProjectionView::addAction(const ActionDispatch &action) {
  int index = -1;
  if (m_actionList.size()) index = m_actionList.rbegin()->first;
  index++;
  m_actionList.insert( ActionListType::value_type( index, action ) );
  return index;
}

void vtkInteractorStyleProjectionView::removeAction(int action) {
  if (action == m_leftButtonAction) 
    CycleLeftButtonAction();
  m_actionList.erase( action );
}

void vtkInteractorStyleProjectionView::activateAction(int action) {
  ActionListType::iterator it = m_actionList.find( action );
  if (it != m_actionList.end()) {
    m_leftButtonAction = action;
    updateLMBHint();
  }
}


/// Destructor
vtkInteractorStyleProjectionView::~vtkInteractorStyleProjectionView() {
  if (m_leftMBHint) m_leftMBHint->Delete();
  if (m_initialState.orientation) m_initialState.orientation->Delete();
  if (m_tempTransform) m_tempTransform->Delete();
}

/** Get Position of Current Event 
  @return true, if successful, false otherwise
*/
bool vtkInteractorStyleProjectionView::GetEventPosition( int &x/**<[out]*/, int &y/**<[out]*/, bool last/**<[in]GetLastEventPosition instead*/ ) {
  vtkRenderWindowInteractor *rwi = this->Interactor;
  if (rwi) {
    const int *pos;
    if (!last)
      pos = rwi->GetEventPosition();
    else
      pos = rwi->GetLastEventPosition();
    if (pos) {
      x = pos[0];
      y = pos[1];
      return true;
    }
  }
  return false;
}

/** Save the state of the Display in order to enable Restricted Actions and ActionCancles*/
void vtkInteractorStyleProjectionView::saveDisplayState(void) {
	m_restriction = None;
	if (m_imageViewer) {
		m_initialState.window = m_imageViewer->GetWindowLevel()->GetWindow();
		m_initialState.level = m_imageViewer->GetWindowLevel()->GetLevel();
	}
	GetEventPosition( m_initialState.mousePosition[0], m_initialState.mousePosition[1]);
	if (m_orientation) m_initialState.orientation->DeepCopy( m_orientation );
}


/** dispatch Actions according to the State of the Mouse Buttons */
void vtkInteractorStyleProjectionView::dipatchActions() {
	saveDisplayState();
	if(!m_stateCtrl)
	{
		if ( m_stateLButton &&  m_stateMButton &&  m_stateRButton) { m_interAction = m_ActionSpin; return; }
		if ( m_stateLButton && !m_stateMButton && !m_stateRButton) { m_interAction = m_ActionNone; return; }
		if ( m_stateLButton &&  m_stateMButton && !m_stateRButton) { m_interAction = m_ActionEmitZoom; return; }
		if ( m_stateLButton && !m_stateMButton &&  m_stateRButton) { m_interAction = m_leftButtonAction; return; }
		if (!m_stateLButton &&  m_stateMButton && !m_stateRButton) { m_interAction = m_ActionPan; return; }
		if (!m_stateLButton &&  m_stateMButton &&  m_stateRButton) { m_interAction = m_ActionRotate; return; }
		if (!m_stateLButton && !m_stateMButton &&  m_stateRButton) { m_interAction = m_ActionSlice; return; }
		if (!m_stateLButton && !m_stateMButton && !m_stateRButton) { m_interAction = m_ActionNone; return; }
	}
	else
	{
		if (!m_stateLButton && !m_stateMButton &&  m_stateRButton) { m_interAction = m_ActionWindowLUT; return; }
		if (!m_stateLButton && !m_stateMButton && !m_stateRButton) { m_interAction = m_ActionShowCircle; return; }
		if ( m_stateLButton && !m_stateMButton && !m_stateRButton) { m_interAction = m_ActionColorPick; return; }
	}
}

/** Cycles through all vtkInteractorStyleProjectionView::InterAction. */
void vtkInteractorStyleProjectionView::CycleLeftButtonAction() {
  ActionListType::iterator it = m_actionList.find( m_leftButtonAction );
  it++;
  if (it == m_actionList.end())
    //it = m_actionList.begin();
	it = m_actionList.find(m_ActionEmitWindowLevel);
  m_leftButtonAction = it->first;
  updateLMBHint();
}

/** Updated the Hint for the Left Mouse Button and starts Animation to fade the hint. */
void vtkInteractorStyleProjectionView::updateLMBHint() {
  if (m_leftButtonAction != m_ActionNone)
    SetLMBHint(1, m_actionList[ m_leftButtonAction ].m_label);
  m_leftMBHintAlpha = 2;
  if (AnimState != VTKIS_ANIM_ON) StartAnimate();
  updateDisplay();
}

/** Update Renderer information, so that vtkInteractorStyleProjectionView::GetCurrentRenderer() actually works*/
void vtkInteractorStyleProjectionView::updateRenderer() {
  int x,y;
  if (GetEventPosition(x,y)) 
    FindPokedRenderer(x,y);
}

/** Sets Left Mouse Button Hint Text and Visibility. 
  @param [in] alpha alpha value (opacity) 0 means invisible, alpha > 1 is clipped to 1
  @param [in] text optional, set new text
  */
void vtkInteractorStyleProjectionView::SetLMBHint( float alpha, const std::string &text ) {
  if ( !m_leftMBHint ) {
    updateRenderer();
    vtkRenderer *ren = GetCurrentRenderer();
    if (ren) {
      m_leftMBHint = vtkTextActor::New();
      ren->AddActor( m_leftMBHint );
    }
  }
  if (m_leftMBHint) {
    if (!text.empty()) m_leftMBHint->SetInput(text.c_str());
    if (alpha <= 0) m_leftMBHint->SetVisibility(false);
    else {
      if (alpha > 1) alpha = 1;
      vtkTextProperty *tp = m_leftMBHint->GetTextProperty();
      tp->SetOpacity( alpha );
      m_leftMBHint->SetTextProperty( tp );
      m_leftMBHint->SetVisibility(true);
    }
  }
}

/** Restrict the Action to major Axis?*/
bool vtkInteractorStyleProjectionView::restrictAction() { 
  if (m_actionList[m_interAction].m_restrict == ActionDispatch::Restricted  && this->Interactor)
    return this->Interactor->GetControlKey();
  return false;
}


/** Gathers Event information and calls the appropiate Action */
void vtkInteractorStyleProjectionView::OnMouseMove()
{
  processAction();
}

void vtkInteractorStyleProjectionView::processAction() {
  if (m_interAction != m_ActionNone) {
    int x,y,ox,oy;
    if (!GetEventPosition(x,y) || !GetEventPosition(ox,oy,true)) return;
    int dx = x - ox;
    int dy = y - oy;
    
    if (m_restriction == XOnly) {
      dy = y - m_initialState.mousePosition[1];
    } else if (m_restriction == YOnly) {
      dx = x - m_initialState.mousePosition[0];
    }
    
	double coord[4] = {0};
	
    if (restrictAction()) {
      int odx = x - m_initialState.mousePosition[0];
      int ody = y - m_initialState.mousePosition[1];
      if (abs(odx) > abs(ody)) {
	m_restriction = XOnly;
	dy -= y - m_initialState.mousePosition[1]; 
      } else {
	m_restriction = YOnly;
	dx -= x - m_initialState.mousePosition[0];
      }
    } else {
      vtkRenderer *ren = //this->GetCurrentRenderer();
      GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
      if (ren) {
	ren->SetDisplayPoint( x, y, 0);// 0.38 );
	ren->DisplayToWorld();
	ren->GetWorldPoint( coord );
	coord[2] = 0.0;
	m_orientation->MultiplyPoint(coord, coord);
      }
    }

	(*m_actionList[m_interAction].m_sig)( dx, dy, coord[0], coord[1], coord[2]);
	updateDisplay();
  }
}


/** Slice forward */
void vtkInteractorStyleProjectionView::OnMouseWheelForward() {
	if(!m_stateCtrl)
		Slice(1);
	else
	{
		m_circle->SetRadius(m_circle->GetRadius()+1);
		updateDisplay();
	}
}

/** Slice backward */
void vtkInteractorStyleProjectionView::OnMouseWheelBackward() {
	if(!m_stateCtrl)
		Slice(-1);
	else if(m_stateCtrl && m_circle->GetRadius() > 1)
	{
		m_circle->SetRadius(m_circle->GetRadius()-1);
		updateDisplay();
	}
}

/** Pan the viewed Object */
void vtkInteractorStyleProjectionView::Pan( int dx/**<[in]*/, int dy/**<[in]*/) {
  if (m_orientation) {
    updateRenderer();
    vtkRenderer *ren = this->GetCurrentRenderer();
    if (ren) {
      double zero[3], d[3];
      zero[0] = 0; zero[1] = 0; zero[2] = 0;
      d[0] = dx; d[1] = dy; d[2] = 0;
      
      ren->SetDisplayPoint( zero );
      ren->DisplayToView();
      ren->GetViewPoint( zero );
      ren->ViewToWorld( zero[0], zero[1], zero[2] );
      
      ren->SetDisplayPoint( d );
      ren->DisplayToView();
      ren->GetViewPoint( d );
      ren->ViewToWorld( d[0], d[1], d[2] );

      m_tempTransform->SetMatrix( m_orientation );
      m_tempTransform->Translate( zero[0] - d[0], zero[1] - d[1], 0 );
      m_tempTransform->GetMatrix( m_orientation );
      updateDisplay();
    }
  }
}

/** update the Display */
void vtkInteractorStyleProjectionView::updateDisplay(void) {
/*
  if (m_orientation->GetMTime() > this->GetInteractor()->GetMTime()) {
    updateRenderExtent();
  }
  */

  if (this->AutoAdjustCameraClippingRange) {
    updateRenderer();
  }
  
	this->GetInteractor()->Render();
}

/** emit ZoomChanged*/
void vtkInteractorStyleProjectionView::EmitZoom(int delta)
{
    emit ZoomChanged(delta);
    Zoom(delta);
}



/** zoom the viewed object*/
void vtkInteractorStyleProjectionView::Zoom( int delta/**<[in] positive numbers mean positive zoom, negative....*/) {
  if (delta && m_orientation) {
    double zoom = 1.0 - delta / 100.0;
    m_tempTransform->SetMatrix( m_orientation );
    m_tempTransform->Scale( zoom, zoom, zoom );
    m_tempTransform->GetMatrix( m_orientation );
    updateDisplay();
  }
}

/** spin the viewed object around the viewing direction */
void vtkInteractorStyleProjectionView::Spin( int angle/**<[in] in degrees*/) {
  if (m_orientation) {
    m_tempTransform->SetMatrix( m_orientation );
    m_tempTransform->RotateZ( angle );
    m_tempTransform->GetMatrix( m_orientation );
    updateDisplay();
  }
}

/** rotate the viewed object around axes perpendicular to the viewing direction*/
void vtkInteractorStyleProjectionView::Rotate( int theta/**<[in] angle around vertical axis*/, int phi/**<[in] angle around horizontal axis*/) {
  if (m_orientation) {
    m_tempTransform->SetMatrix( m_orientation );
    m_tempTransform->RotateX( phi );
    m_tempTransform->RotateY( -theta );
    m_tempTransform->GetMatrix( m_orientation );
    updateDisplay();
  }
}

/** slice the viewed object */
void vtkInteractorStyleProjectionView::Slice(int dpos/**<[in] delta in position perpendicular to the viewing direction*/) {
  
if (m_orientation) {
    m_tempTransform->SetMatrix( m_orientation );
    m_tempTransform->Translate(0, 0, dpos* m_sliceIncrement);
    m_tempTransform->GetMatrix( m_orientation );
    updateDisplay();
  }
  
}

/** emit change Window and Level */
void vtkInteractorStyleProjectionView::EmitWindowLevelDelta(int dw, int dl)
{
    emit WindowLevelDeltaChanged(dw, dl);
    WindowLevelDelta(dw, dl);
}


/** change Window and Level */
void vtkInteractorStyleProjectionView::WindowLevelDelta( int dw/**<[in] delta window*/, int dl/**<[in] delta level*/) {
	if (m_imageViewer && (dw || dl)) 
	{
		dw += m_imageViewer->GetWindowLevel()->GetWindow();
		dl += m_imageViewer->GetWindowLevel()->GetLevel();;
		if (dw) 
		{
			m_imageViewer->SetColorWindow(dw);
		}
		if (dl) 
		{
			m_imageViewer->SetColorLevel(dl);
		}
		updateDisplay();
	}
}

void vtkInteractorStyleProjectionView::WindowLUTDelta( int dw/**<[in] delta window*/, int dl/**<[in] delta level*/) {
	if (m_colorMap && m_imageViewer && (dw || dl)) 
	{
		double *range = m_colorMap->GetRange();
		
		double ddw, ddl;

		ddw = (double)dw / (double)m_imageViewer->GetSize()[0] * 10;
		ddl = (double)dl / (double)m_imageViewer->GetSize()[0] * 10;

		if(range[0] - ddw < range[1] + ddw)
			range[0] -= ddw;
		if(range[1] + ddw > range[0] - ddw)
			range[1] += ddw;
		
		range[0] += ddl;
		range[1] += ddl;

		if(range)
		{
			m_colorMap->SetRange(range);
			m_colorMap->ForceBuild();

			//create alpha ramp at the borders 
			for(int i = 0; i < 16; i++)
			{
				double rgba[4];
				m_colorMap->GetTableValue(i,rgba);
				m_colorMap->SetTableValue(i,rgba[0],rgba[1],rgba[2],1/16*i);

				m_colorMap->GetTableValue(255-i,rgba);
				m_colorMap->SetTableValue(255-i,rgba[0],rgba[1],rgba[2],1/16*i);
			}

			//m_imageViewer->Render();

			//std::cout << "range[0]= " << range[0] << "; range[1]= " << range[1] << std::endl;
		}

		emit ColorTableChanged();

		updateDisplay();
	}
}

void vtkInteractorStyleProjectionView::OnLeftButtonDown()
{
  m_stateLButton = true;
  dipatchActions();
  if (m_interAction != m_ActionNone)
    if (m_actionList[ m_interAction ].m_atype == ActionDispatch::ClickingAction)
      processAction();
}

void vtkInteractorStyleProjectionView::OnLeftButtonUp()
{
  m_stateLButton = false;
  dipatchActions();
}

void vtkInteractorStyleProjectionView::OnMiddleButtonDown()
{
  m_stateMButton = true;
  dipatchActions();
}

void vtkInteractorStyleProjectionView::OnMiddleButtonUp()
{
  m_stateMButton = false;
  dipatchActions();
}

void vtkInteractorStyleProjectionView::OnRightButtonDown()
{
  m_stateRButton = true;
  dipatchActions();
}

void vtkInteractorStyleProjectionView::OnRightButtonUp()
{
  m_stateRButton = false;
  dipatchActions();
}

/** 
  - change Left Interaction Mode on Space.
  - rotate 90 degress when [CTRL] + [Cursor Key] is pressed
  */
void vtkInteractorStyleProjectionView::OnKeyDown()
{
	m_stateCtrl = this->GetInteractor()->GetControlKey();

  //const char *keySym = this->GetInteractor()->GetKeySym();
	std::string keySym = this->GetInteractor()->GetKeySym();

	if (!m_stateCtrl) {
		if (keySymLeft.compare(keySym) == 0) { Rotate(90,0); return; }
		if (keySymRight.compare(keySym) == 0) { Rotate(-90,0); return; }
		if (keySymUp.compare(keySym) == 0) { Rotate(0,90); return; }
		if (keySymDown.compare(keySym) == 0) { Rotate(0,-90); return; }
  
		if (keySymSpace.compare(keySym) == 0) { CycleLeftButtonAction(); return; }
	} else
	{
		m_imageViewer->GetRenderer()->AddActor(m_circleActor);
		dipatchActions();
	}


  cerr << __FILE__ << "[" << __LINE__ << "]:" << __FUNCTION__ << " Code:" << (int)this->GetInteractor()->GetKeyCode() << " Sym:" << this->GetInteractor()->GetKeySym() << endl;
}

void vtkInteractorStyleProjectionView::OnKeyUp()
{
	if(m_stateCtrl)
	{
		m_stateCtrl = false;
		m_interAction = m_ActionNone;
		m_annotation->SetText(0, "");
			m_imageViewer->GetRenderer()->RemoveActor(m_circleActor);
		updateDisplay();
	}

}

/** animate the Left Mouse Button hint*/
void vtkInteractorStyleProjectionView::OnTimer() 
{
  vtkInteractorStyle::OnTimer();
  if (m_leftMBHintAlpha > 0) {
    m_leftMBHintAlpha -= 0.5 / this->GetInteractor()->GetDesiredUpdateRate();
    if (m_leftMBHintAlpha >= 1) return;
    if (m_leftMBHintAlpha <= 0) 
      SetLMBHint( 0 );
    else {
      SetLMBHint( m_leftMBHintAlpha );
    }
    updateDisplay();
  } else {
    StopAnimate();
  }
}

void vtkInteractorStyleProjectionView::SetAnnotation(vtkCornerAnnotation* annotation)
{
	m_annotation = annotation;
	if(m_imageViewer)
	{
		m_imageViewer->GetRenderer()->AddViewProp(m_annotation);
	}
}

void vtkInteractorStyleProjectionView::ShowCircle()
{
	int x_display, y_display;

	// Pick at the mouse location provided by the interactor
	double pos_world[4];
	double pos_perfusion[4] = {0};

	if (GetEventPosition( x_display, y_display )) 
	{
		vtkRenderer *ren = //this->GetCurrentRenderer();
			GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
		if (ren) {
			ren->SetDisplayPoint( x_display, y_display, 0);// 0.38 );
			ren->DisplayToWorld();
			ren->GetWorldPoint( pos_world );
			pos_world[2] = 0.0;
			m_orientation->MultiplyPoint(pos_world, pos_perfusion);

			std::cout << "Pick position (world coordinates) is: "
                << pos_world[0] << " " << pos_world[1]
                << " " << pos_world[2] << std::endl;
		
			m_circle->SetCenter(pos_world[0], pos_world[1], pos_world[2]);
		}
	}
}

void vtkInteractorStyleProjectionView::PickPerfusionValues()
{

	vtkSmartPointer<vtkPNGWriter> writer =
		vtkSmartPointer<vtkPNGWriter>::New();

	vtkSmartPointer<vtkImageShiftScale> scale = 
		vtkSmartPointer<vtkImageShiftScale>::New();

	if(m_Overlay)
	{

		scale->SetInput(m_Overlay->getReslice()->GetOutput());
		scale->SetOutputScalarTypeToUnsignedShort();

		writer->SetFileName("resliceTest.png");
		writer->SetInput(scale->GetOutput());
		writer->Write();
	}

	int x_display, y_display;

	int perfusionIndex;
	// Pick at the mouse location provided by the interactor
	double pos_world[4];
	double pos_perfusion[4] = {0};

	if (GetEventPosition( x_display, y_display )) 
	{
		vtkRenderer *ren = GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer();

		if (ren) {
			ren->SetDisplayPoint( x_display, y_display, 0);// 0.38 );
			ren->DisplayToWorld();
			ren->GetWorldPoint( pos_world );
			pos_world[2] = 0.0;
			m_orientation->MultiplyPoint(pos_world, pos_perfusion);

			std::cout << "Pick position (world coordinates) is: "
				<< pos_world[0] << " " << pos_world[1]
			<< " " << pos_world[2] << std::endl;

			m_circle->SetCenter(pos_world[0], pos_world[1], pos_world[2]);
			double* bounds_world = m_circleActor->GetBounds();

			double start[2] = {0};
			double end[2] = {0};

			start[0] = bounds_world[0]; start[1] = bounds_world[2];
			end[0] = bounds_world[1]; end[1] = bounds_world[3];


			vtkSmartPointer<vtkSelectEnclosedPoints> selectEnclosedPoints = 
				vtkSmartPointer<vtkSelectEnclosedPoints>::New();

			selectEnclosedPoints->Initialize(m_circle->GetOutput());
			selectEnclosedPoints->SetTolerance(0.1);

			double mean = 0;
			int count = 0;

			float* perfusionValues;

			switch (m_Overlay->getReslice()->GetOutput()->GetNumberOfScalarComponents()) {
			case 1:
				{ 
					perfusionValues = (float*)m_Overlay->getReslice()->GetOutput()->GetScalarPointer();
				}
				break;
			}

			//create binary overlay

			MaxSlopeAnalyzer* maxSlopeAnalyzer = new MaxSlopeAnalyzer(NULL);

			int childCount = m_rootItem->childCount();
			CTImageTreeItem* castItem = new CTImageTreeItem();

			bool cast = false;
			for (int i = 0; i < childCount; i++)
			{
				TreeItem* currentItem = &m_rootItem->child(i);
				if(currentItem->isA(typeid(CTImageTreeItem)))
				{
					maxSlopeAnalyzer->addImage( dynamic_cast<CTImageTreeItem*>(currentItem) );
					if(!cast)
					{
						castItem = dynamic_cast<CTImageTreeItem*>(currentItem);
						cast = true;
					}

					int n = currentItem->childCount();

					for( int j = 0; j < n; j++)
					{
						TreeItem* currentChild = &currentItem->child(j);
						if(currentChild->isA(typeid(BinaryImageTreeItem)))
						{
							m_arterySegment = dynamic_cast<BinaryImageTreeItem*>(currentChild);
						}
					}
				}
			}

			BinaryImageTreeItem::ImageType::Pointer seg;
			typedef itk::CastImageFilter< CTImageType, BinaryImageType> CastFilterType;

			CastFilterType::Pointer caster = CastFilterType::New();
			caster->SetInput( castItem->getITKImage() );
			caster->Update();
			seg = caster->GetOutput();
			//fills the segment with zeros
			seg->FillBuffer(BinaryPixelOff);

			BinaryImageTreeItem *result = new BinaryImageTreeItem(m_rootItem, seg, "ROI");
			result->drawSphere(m_circle->GetRadius(), pos_perfusion[0], pos_perfusion[1], pos_perfusion[2], false);
			
			//result->drawPlate(m_circle->GetRadius(),pos_perfusion[0], pos_perfusion[1], pos_perfusion[2], m_orientation,false);

			maxSlopeAnalyzer->addSegment(result);
			maxSlopeAnalyzer->addSegment(m_arterySegment);
			
			maxSlopeAnalyzer->calculateTacValues();
			SegmentListModel *segments = maxSlopeAnalyzer->getSegments();

			//picker = new TimeDensityDataPicker(markerPickerX, markerPickerY, segments, this->ui->qwtPlot_tac->canvas());

			double perfusion = 0;


			//iterate over the list of segments
			BOOST_FOREACH( SegmentInfo &currentSegment, *segments) {
				
				currentSegment.setEnableGamma(true);
				currentSegment.setGammaStartIndex(0);
				currentSegment.setGammaEndIndex(childCount);
					//attach the curves for the actual segment to the plot
				//currentSegment.attachSampleCurves(m_plot);
				currentSegment.setArterySegment(&maxSlopeAnalyzer->getSegments()->getSegment(1));
				maxSlopeAnalyzer->recalculateGamma(currentSegment);
			}

			maxSlopeAnalyzer->getSegments()->getSegment(0).attachSampleCurves(m_plot);

			m_plot->legend()->contentsWidget()->setVisible(false);
			m_plot->replot();

			//maxSlopeAnalyzer->popBackSegment();
	//		m_arterySegment->setEnableGamma(true);
	//		m_arterySegment->setGammaStartIndex(0);
	//		m_arterySegment->setGammaEndIndex(childCount);
	//		maxSlopeAnalyzer->recalculateGamma(*m_arterySegment);

			perfusion = 60 * maxSlopeAnalyzer->getSegments()->getSegment(0).getGammaMaxSlope() / maxSlopeAnalyzer->getSegments()->getSegment(0).getArterySegment()->getGammaMaximum();

			ofstream resultFile;
			resultFile.open ("tempMeasuredResults.csv");
			resultFile << maxSlopeAnalyzer->getTacValuesAsString() << std::endl;
			resultFile << "Perfusion;" << perfusion << std::endl;
			resultFile << "Max Slope;" << maxSlopeAnalyzer->getSegments()->getSegment(0).getGammaMaxSlope() << std::endl;
			resultFile << "Max Gamma;" << maxSlopeAnalyzer->getSegments()->getSegment(0).getArterySegment()->getGammaMaximum() << std::endl;
			
			resultFile.close();

			//iterate over ROI

/*			double* spac = m_Overlay->getReslice()->GetOutput()->GetSpacing();

			for(double y = start[1]; y < end[1];y+=spac[1])
				for(double x = start[0]; x < end[0];x+=spac[0])
				{
					if(selectEnclosedPoints->IsInsideSurface(x,y,0)) 
					{
						pos_world[0] = x;
						pos_world[1] = y;

						int idx = m_Overlay->getReslice()->GetOutput()->FindPoint(pos_world);

						if(idx >= 0)
						{
							float p = perfusionValues[idx];
							mean += p;
							count++;
						}
						std::cout << x << ", " << y << ", " << "0" << std::endl;
					}
				}
*/
				updateDisplay();
				std::stringstream message;

				message << "Perfusion = [" << perfusion << "]"; //mean/(double)count << "]";


				m_annotation->SetText( 0, message.str().c_str() );
		}
	}

	m_interAction = m_ActionNone;
	//m_annotation->SetText(0, "");
	m_imageViewer->GetRenderer()->RemoveActor(m_circleActor);
	updateDisplay();
	m_stateLButton = false;
	m_stateCtrl = false;
	dipatchActions();
}
