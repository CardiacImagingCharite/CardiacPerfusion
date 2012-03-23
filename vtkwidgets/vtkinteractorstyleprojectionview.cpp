/*
    This file is part of perfusionkit.
    Copyright 2010 Henning Meyer

    perfusionkit is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    perfusionkit is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with perfusionkit.  If not, see <http://www.gnu.org/licenses/>.
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

#include <string>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>

#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>

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
  ActionNone(-1),
  m_stateRButton(false),
  m_stateLButton(false),
  m_stateMButton(false),
  m_sliceIncrement(1.0),
  m_leftMBHint( NULL ),
  m_imageMapToWindowLevelColors(NULL),
  m_orientation(NULL),
  tempTransform( vtkTransform::New() )
{
  State = VTKIS_NONE;
  UseTimers = 1;
  SetLMBHint(0);
  m_initialState.orientation = vtkMatrix4x4::New();
  resetActions();
  tempTransform->PreMultiply();
}

void vtkInteractorStyleProjectionView::resetActions() {
  ActionFirst = ActionSlice = addAction("Slice", boost::bind(&vtkInteractorStyleProjectionView::Slice, this, _2), ActionDispatch::MovingAction, ActionDispatch::UnRestricted );
  ActionRotate = addAction("Rotate", boost::bind(&vtkInteractorStyleProjectionView::Rotate, this, _1, _2), ActionDispatch::MovingAction, ActionDispatch::Restricted );
  ActionSpin = addAction("Spin", boost::bind(&vtkInteractorStyleProjectionView::Spin, this, _1), ActionDispatch::MovingAction, ActionDispatch::UnRestricted );
  ActionZoom = addAction("Zoom", boost::bind(&vtkInteractorStyleProjectionView::Zoom, this, _2 ), ActionDispatch::MovingAction, ActionDispatch::UnRestricted );
  ActionPan = addAction("Pan", boost::bind(&vtkInteractorStyleProjectionView::Pan, this, _1, _2), ActionDispatch::MovingAction, ActionDispatch::Restricted );
  ActionWindowLevel = addAction("Window/Level", boost::bind(&vtkInteractorStyleProjectionView::WindowLevelDelta, this, _1, _2), ActionDispatch::MovingAction, ActionDispatch::Restricted );
  m_leftButtonAction = ActionSlice;
  m_interAction = ActionNone;
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
  if (tempTransform) tempTransform->Delete();
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
  restriction = None;
  if (m_imageMapToWindowLevelColors) {
    m_initialState.window = m_imageMapToWindowLevelColors->GetWindow();
    m_initialState.level = m_imageMapToWindowLevelColors->GetLevel();
  }
  GetEventPosition( m_initialState.mousePosition[0], m_initialState.mousePosition[1]);
  if (m_orientation) m_initialState.orientation->DeepCopy( m_orientation );
}


/** dispatch Actions according to the State of the Mouse Buttons */
void vtkInteractorStyleProjectionView::dipatchActions() {
  saveDisplayState();
  if ( m_stateLButton &&  m_stateMButton &&  m_stateRButton) { m_interAction = ActionSpin; return; }
  if ( m_stateLButton &&  m_stateMButton && !m_stateRButton) { m_interAction = ActionRotate; return; }
  if ( m_stateLButton && !m_stateMButton &&  m_stateRButton) { m_interAction = ActionZoom; return; }
  if ( m_stateLButton && !m_stateMButton && !m_stateRButton) { m_interAction = m_leftButtonAction; return; }
  if (!m_stateLButton &&  m_stateMButton &&  m_stateRButton) { m_interAction = ActionPan; return; }
  if (!m_stateLButton &&  m_stateMButton && !m_stateRButton) { m_interAction = ActionWindowLevel; return; }
  if (!m_stateLButton && !m_stateMButton &&  m_stateRButton) { m_interAction = ActionSlice; return; }
  if (!m_stateLButton && !m_stateMButton && !m_stateRButton) { m_interAction = ActionNone; return; }
}

/** Cycles through all vtkInteractorStyleProjectionView::InterAction. */
void vtkInteractorStyleProjectionView::CycleLeftButtonAction() {
  ActionListType::iterator it = m_actionList.find( m_leftButtonAction );
  it++;
  if (it == m_actionList.end())
    it = m_actionList.begin();
  m_leftButtonAction = it->first;
  updateLMBHint();
}

/** Updated the Hint for the Left Mouse Button and starts Animation to fade the hint. */
void vtkInteractorStyleProjectionView::updateLMBHint() {
  if (m_leftButtonAction != ActionNone)
    SetLMBHint(1, m_actionList[ m_leftButtonAction ].label);
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
  if (m_actionList[m_interAction].restrict == ActionDispatch::Restricted  && this->Interactor)
    return this->Interactor->GetControlKey();
  return false;
}


/** Gathers Event information and calls the appropiate Action */
void vtkInteractorStyleProjectionView::OnMouseMove()
{
  processAction();
}

void vtkInteractorStyleProjectionView::processAction() {
  if (m_interAction != ActionNone) {
    int x,y,ox,oy;
    if (!GetEventPosition(x,y) || !GetEventPosition(ox,oy,true)) return;
    int dx = x - ox;
    int dy = y - oy;
    
    if (restriction == XOnly) {
      dy = y - m_initialState.mousePosition[1];
    } else if (restriction == YOnly) {
      dx = x - m_initialState.mousePosition[0];
    }
    
	double coord[4] = {0};
	
    if (restrictAction()) {
      int odx = x - m_initialState.mousePosition[0];
      int ody = y - m_initialState.mousePosition[1];
      if (abs(odx) > abs(ody)) {
	restriction = XOnly;
	dy -= y - m_initialState.mousePosition[1]; 
      } else {
	restriction = YOnly;
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

	(*m_actionList[m_interAction].sig)( dx, dy, coord[0], coord[1], coord[2]);
	updateDisplay();
  }
}


/** Slice forward */
void vtkInteractorStyleProjectionView::OnMouseWheelForward() {
  Slice(1);
}

/** Slice backward */
void vtkInteractorStyleProjectionView::OnMouseWheelBackward() {
  Slice(-1);
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

      tempTransform->SetMatrix( m_orientation );
      tempTransform->Translate( zero[0] - d[0], zero[1] - d[1], 0 );
      tempTransform->GetMatrix( m_orientation );
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


/** zoom the viewed object*/
void vtkInteractorStyleProjectionView::Zoom( int delta/**<[in] positive numbers mean positive zoom, negative....*/) {
  if (delta && m_orientation) {
    double zoom = 1.0 - delta / 100.0;
    tempTransform->SetMatrix( m_orientation );
    tempTransform->Scale( zoom, zoom, zoom );
    tempTransform->GetMatrix( m_orientation );
    updateDisplay();
  }
}

/** spin the viewed object around the viewing direction */
void vtkInteractorStyleProjectionView::Spin( int angle/**<[in] in degrees*/) {
  if (m_orientation) {
    tempTransform->SetMatrix( m_orientation );
    tempTransform->RotateZ( angle );
    tempTransform->GetMatrix( m_orientation );
    updateDisplay();
  }
}

/** rotate the viewed object around axes perpendicular to the viewing direction*/
void vtkInteractorStyleProjectionView::Rotate( int theta/**<[in] angle around vertical axis*/, int phi/**<[in] angle around horizontal axis*/) {
  if (m_orientation) {
    tempTransform->SetMatrix( m_orientation );
    tempTransform->RotateX( phi );
    tempTransform->RotateY( -theta );
    tempTransform->GetMatrix( m_orientation );
    updateDisplay();
  }
}

/** slice the viewed object */
void vtkInteractorStyleProjectionView::Slice(int dpos/**<[in] delta in position perpendicular to the viewing direction*/) {
  if (m_orientation) {
    tempTransform->SetMatrix( m_orientation );
    tempTransform->Translate(0, 0, dpos* m_sliceIncrement);
    tempTransform->GetMatrix( m_orientation );
    updateDisplay();
  }
}

/** change Window and Level */
void vtkInteractorStyleProjectionView::WindowLevelDelta( int dw/**<[in] delta window*/, int dl/**<[in] delta level*/) {
  if (m_imageMapToWindowLevelColors  && (dw || dl)) {
      dw += m_imageMapToWindowLevelColors->GetWindow();
      dl += m_imageMapToWindowLevelColors->GetLevel();
    if (dw) m_imageMapToWindowLevelColors->SetWindow(dw);
    if (dl) m_imageMapToWindowLevelColors->SetLevel(dl);
    updateDisplay();
  }
}


void vtkInteractorStyleProjectionView::OnLeftButtonDown()
{
  m_stateLButton = true;
  dipatchActions();
  if (m_interAction != ActionNone)
    if (m_actionList[ m_interAction ].atype == ActionDispatch::ClickingAction)
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
  const char *keySym = this->GetInteractor()->GetKeySym();
  if (this->GetInteractor()->GetControlKey()) {
    if (keySymLeft == keySym) { Rotate(90,0); return; }
    if (keySymRight == keySym) { Rotate(-90,0); return; }
    if (keySymUp == keySym) { Rotate(0,90); return; }
    if (keySymDown == keySym) { Rotate(0,-90); return; }
  }
  if (keySymSpace == keySym) { CycleLeftButtonAction(); return; }
  cerr << __FILE__ << "[" << __LINE__ << "]:" << __FUNCTION__ << " Code:" << (int)this->GetInteractor()->GetKeyCode() << " Sym:" << this->GetInteractor()->GetKeySym() << endl;
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
