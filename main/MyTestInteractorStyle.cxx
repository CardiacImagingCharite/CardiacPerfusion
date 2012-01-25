// Define interaction style

#include "MyTestInteractorStyle.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include <string>

	MyTestInteractorStyle::MyTestInteractorStyle()
	{
		m_startPoint = vtkSmartPointer<vtkCoordinate>::New();
		m_endPoint = vtkSmartPointer<vtkCoordinate>::New();
		m_mouseMovement = vtkSmartPointer<vtkCoordinate>::New();
		m_region = vtkSmartPointer<vtkRegularPolygonSource>::New();

		m_leftMouseButtonDown = false;
	    m_rightMouseButtonDown = false;
	    m_middleMouseButtonDown = false;
	}

	MyTestInteractorStyle::~MyTestInteractorStyle()
	{
	
	}

    void MyTestInteractorStyle::OnKeyPress() 
    {
      // Get the keypress
      vtkRenderWindowInteractor *rwi = this->Interactor;
      std::string key = rwi->GetKeySym();

      // Output the key that was pressed
      std::cout << "Pressed " << key << std::endl;
 
      // Handle an arrow key
      if(key == "Up")
        {
        std::cout << "The up arrow was pressed." << std::endl;
        }
 
      // Handle a "normal" key
      if(key == "a")
        {
        std::cout << "The a key was pressed." << std::endl;
		m_viewer->SetSliceOrientation((m_viewer->GetSliceOrientation()+1)%3);
        }
 
      // Forward events
      vtkInteractorStyleTrackballCamera::OnKeyPress();
    }

	void MyTestInteractorStyle::selectUserInteraction()
	{
		     if( m_leftMouseButtonDown && !m_middleMouseButtonDown &&  m_rightMouseButtonDown) goThroughVolume();
		else if( m_leftMouseButtonDown && !m_middleMouseButtonDown && !m_rightMouseButtonDown) selectRegion();
		else if(!m_leftMouseButtonDown &&  m_middleMouseButtonDown && !m_rightMouseButtonDown) Spin();
		else if(!m_leftMouseButtonDown &&  m_middleMouseButtonDown &&  m_rightMouseButtonDown) Rotate();
		else if( m_leftMouseButtonDown &&  m_middleMouseButtonDown && !m_rightMouseButtonDown) Dolly();
		else if(!m_leftMouseButtonDown && !m_middleMouseButtonDown &&  m_rightMouseButtonDown) Pan();
	}

	void MyTestInteractorStyle::goThroughVolume()
	{
		m_viewer->SetSlice(m_viewer->GetSlice() - m_mouseMovement->GetValue()[1]);
	}

	void MyTestInteractorStyle::selectRegion()
	{
	  //polygonSource->GeneratePolygonOff();
	  m_region->SetNumberOfSides(50);
	  m_region->SetRadius((this->Interactor->GetEventPosition()[0] - m_startPoint->GetValue()[0]) / 2);
	  m_region->SetCenter((this->Interactor->GetEventPosition()[0] + m_startPoint->GetValue()[0]) / 2,
						   (this->Interactor->GetEventPosition()[1] + m_startPoint->GetValue()[1]) / 2, 
						    0);
	  m_region->Update();
	  m_viewer->Render();
	}

 //catch Interaction-Events

	void MyTestInteractorStyle::OnMouseMove()
	{
		int* posOld = this->Interactor->GetLastEventPosition();
		int* posNew = this->Interactor->GetEventPosition();
		m_mouseMovement->SetValue(posNew[0]-posOld[0], 
									posNew[1]-posOld[1]);

		cout << "x: " << m_mouseMovement->GetValue()[0] << ", y: " << m_mouseMovement->GetValue()[1] << endl;
		selectUserInteraction();
		vtkInteractorStyleTrackballCamera::OnMouseMove();
	}

	void MyTestInteractorStyle::OnMouseWheelForward()
	{
		m_viewer->SetSlice(m_viewer->GetSlice()+1);
		m_viewer->Render();
	}
	
	void MyTestInteractorStyle::OnMouseWheelBackward()
	{
		m_viewer->SetSlice(m_viewer->GetSlice()-1);
		m_viewer->Render();
	}

	void MyTestInteractorStyle::OnLeftButtonDown()
	{
		m_leftMouseButtonDown = true;

		//set StartPoint of Interaction
		setStartPoint(this->Interactor->GetEventPosition());
		
		selectUserInteraction();
	
		//vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
	}

	void MyTestInteractorStyle::OnLeftButtonUp()
	{
		m_leftMouseButtonDown = false;
		//vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
		setEndPoint(this->Interactor->GetEventPosition());

		selectUserInteraction();
	}

	void MyTestInteractorStyle::OnRightButtonDown()
	{
		m_rightMouseButtonDown = true;

		//set StartPoint of Interaction
		setStartPoint(this->Interactor->GetEventPosition());

		selectUserInteraction();

		//vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
	}

	void MyTestInteractorStyle::OnRightButtonUp()
	{
		m_rightMouseButtonDown = false;
		setEndPoint(this->Interactor->GetEventPosition());

		selectUserInteraction();
		//vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
	}

	void MyTestInteractorStyle::OnMiddleButtonDown()
	{
		m_middleMouseButtonDown = true;

		//set StartPoint of Interaction
		setStartPoint(this->Interactor->GetEventPosition());

		selectUserInteraction();
		//vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
	}

	void MyTestInteractorStyle::OnMiddleButtonUp()
	{
		m_middleMouseButtonDown = false;
		setEndPoint(this->Interactor->GetEventPosition());

		selectUserInteraction();
		//vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
	}

	void MyTestInteractorStyle::setImageViewer(vtkSmartPointer<vtkImageViewer2> viewer)
	{
		this->m_viewer = viewer;

		vtkSmartPointer<vtkPolyDataMapper> mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputConnection(m_region->GetOutputPort());
		vtkSmartPointer<vtkActor> actor =
			vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);

		m_viewer->GetRenderer()->AddActor(actor);
	}

	void MyTestInteractorStyle::setStartPoint(int* p)
	{
		m_startPoint->SetValue(p[0], p[1], p[2]);
	}

	void MyTestInteractorStyle::setEndPoint(int* p)
	{
		m_endPoint->SetValue(p[0], p[1], p[2]);
	}