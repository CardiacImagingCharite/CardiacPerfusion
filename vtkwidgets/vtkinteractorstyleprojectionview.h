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

#ifndef VTKINTERACTORSTYLEPROJECTIONVIEW_H
#define VTKINTERACTORSTYLEPROJECTIONVIEW_H

#include <vtkInteractorStyle.h>
#include "vtkinteractoractiondispatch.h"


class vtkImageMapToWindowLevelColors;
class vtkMatrix4x4;
class vtkTextActor;
class vtkTransform;


/// Interactor Style specific for Projection Views.
/** This class should be used as interaction style for projection views.
It features mouse interaction with combination of Mouse Button presses 
as well as selection of interaction mode by pressing Space.
*/

class vtkInteractorStyleProjectionView : public vtkInteractorStyle
{
  vtkTypeRevisionMacro(vtkInteractorStyleProjectionView,vtkInteractorStyle);
  public:
  /// Default Construction Method filled by vtkStandardNewMacro
  static vtkInteractorStyleProjectionView *New();

  vtkInteractorStyleProjectionView();
  ~vtkInteractorStyleProjectionView();
  
  /** @name Overidden Event Handlers
      inherited from vtkInteractorStyle */
  ///@{
  virtual void OnMouseMove();
  virtual void OnLeftButtonDown();
  virtual void OnLeftButtonUp();
  virtual void OnMiddleButtonDown();
  virtual void OnMiddleButtonUp();
  virtual void OnRightButtonDown();
  virtual void OnRightButtonUp();
  virtual void OnKeyDown();
  virtual void OnMouseWheelForward();
  virtual void OnMouseWheelBackward();
  virtual void OnTimer();
  ///@}
  
  
  /** @name Attribute Setters
      in order to give vtkInteractorStyleProjectionView needed access to the displayed entities */
  ///@{
  void SetImageMapToWindowLevelColors(vtkImageMapToWindowLevelColors *map/**<[in]*/) { m_imageMapToWindowLevelColors = map; }
  void SetOrientationMatrix(vtkMatrix4x4 *orientation/**<[in]*/) { m_orientation = orientation; }
  ///@}
  void CycleLeftButtonAction();
  void WindowLevelDelta( int dw, int dl );
  void Slice( int dpos );
  void Rotate( int theta, int phi );
  void Spin( int alpha );
  void Zoom( int delta );
  void Pan( int dx, int dy );
  int addAction(const std::string &label, const ActionSignal::slot_type &slot,
    ActionDispatch::ActionType atype, ActionDispatch::RestrictionType restrict);
  int addAction(const ActionDispatch &action);
  void activateAction(int action);
  void removeAction(int action);
  void resetActions();
  protected:
  void processAction();
  void updateRenderer();
  void dipatchActions();
  void updateLMBHint();
  void SetLMBHint( float alpha, const std::string &text = std::string() );
  bool GetEventPosition( int &x, int &y, bool last=false );
  bool restrictAction();
  void saveDisplayState(void);
  void updateDisplay(void);
  
  struct DisplayState {
    int window;
    int level;
    int mousePosition[2];
    vtkMatrix4x4 *orientation;
  };
  
  enum Restriction {
    XOnly,
    YOnly,
    None
  };
  Restriction restriction;
  typedef std::map< int, ActionDispatch > ActionListType;
  ActionListType m_actionList;
  int m_interAction; ///< selected interaction due to mouse button presses - determined by dipatchActions()
  int m_leftButtonAction; ///< selected interaction for the left mouse button - changed by pressing Space in CycleLeftButtonAction()
  int ActionFirst, ActionSpin, ActionRotate, ActionZoom, ActionPan, ActionWindowLevel, ActionSlice, ActionNone;
  
  /** @name Mouse Button Flags
      State of the Mouse Buttons (Pressed?) */
  //@{
  bool m_stateRButton; 
  bool m_stateLButton;
  bool m_stateMButton;
  //@}
  float m_sliceIncrement; ///< Value to Increment the Viewers Z-Position when slicing
  vtkTextActor *m_leftMBHint; ///< Hint actor for showing the Action associated with the Left Mouse Button
  float m_leftMBHintAlpha; ///< alpha value for #m_leftMBHint
  vtkImageMapToWindowLevelColors *m_imageMapToWindowLevelColors; ///< Mapper (set external via SetImageMapToWindowLevelColors()) for Window and Level (changed via vtkInteractorStyleProjectionView::ActionWindowLevel)
  vtkMatrix4x4 *m_orientation; ///< the Transformation Matrix of the displayed Data
  DisplayState m_initialState; ///< Display state at the beginning of an action
  private:
  vtkTransform *tempTransform;
};

#endif // VTKINTERACTORSTYLEPROJECTIONVIEW_H
