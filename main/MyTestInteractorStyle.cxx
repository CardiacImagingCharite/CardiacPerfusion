// Define interaction style

#include "MyTestInteractorStyle.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include <string>

#include "vtkPolyLine.h"
#include "vtkCellArray.h"
#include "vtkPoints.h"

	MyTestInteractorStyle::MyTestInteractorStyle()
	{
		m_startPoint = vtkSmartPointer<vtkCoordinate>::New();
		m_endPoint = vtkSmartPointer<vtkCoordinate>::New();
		m_mouseMovement = vtkSmartPointer<vtkCoordinate>::New();
		m_region = vtkSmartPointer<vtkPolyData>::New();
		m_roiPoints = vtkSmartPointer<vtkPoints>::New();

		m_leftMouseButtonDown = false;
	    m_rightMouseButtonDown = false;
	    m_middleMouseButtonDown = false;
		m_painting = false;
	}

	MyTestInteractorStyle::~MyTestInteractorStyle()
	{
	
	}

	void MyTestInteractorStyle::selectUserInteraction()
	{
		     if( m_leftMouseButtonDown && !m_middleMouseButtonDown &&  m_rightMouseButtonDown) goThroughVolume();
		else if( m_leftMouseButtonDown && !m_middleMouseButtonDown && !m_rightMouseButtonDown) windowLevel();
		else if(!m_leftMouseButtonDown &&  m_middleMouseButtonDown && !m_rightMouseButtonDown) Spin();
		else if(!m_leftMouseButtonDown &&  m_middleMouseButtonDown &&  m_rightMouseButtonDown) Rotate();
		else if( m_leftMouseButtonDown &&  m_middleMouseButtonDown && !m_rightMouseButtonDown) Dolly();
		else if(!m_leftMouseButtonDown && !m_middleMouseButtonDown &&  m_rightMouseButtonDown) Pan();

	}

	void MyTestInteractorStyle::windowLevel()
	{
		m_viewer->SetColorLevel(m_viewer->GetColorLevel() - m_mouseMovement->GetValue()[0]);
		m_viewer->SetColorWindow(m_viewer->GetColorWindow() - m_mouseMovement->GetValue()[1]);
		m_viewer->Render();
	}

	void MyTestInteractorStyle::goThroughVolume()
	{
		m_viewer->SetSlice(m_viewer->GetSlice() - m_mouseMovement->GetValue()[1]);
		m_viewer->Render();
	}

	void MyTestInteractorStyle::paintRegion()
	{

	}

 //catch Interaction-Events

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

	  else if(key == "p")
	  {
		m_painting = !m_painting;
		if(m_painting)
			cout << "Start painting" << endl;
		else
			cout << "Stop painting" << endl;
	  }
 
      // Forward events
      //vtkInteractorStyleTrackballCamera::OnKeyPress();
    }


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

		if(m_painting)
		{
			m_roiPoints->InsertNextPoint(m_endPoint->GetValue());
		
			vtkSmartPointer<vtkPolyLine> polyLine = 
				vtkSmartPointer<vtkPolyLine>::New();

			int id = m_roiPoints->GetNumberOfPoints();
			if(id > 1)
			{
				polyLine->GetPointIds()->SetNumberOfIds(id);
			
				for(int i = 0; i < id; i++)
					polyLine->GetPointIds()->SetId(id,id);
		
				// Create a cell array to store the lines in and add the lines to it
				vtkSmartPointer<vtkCellArray> cells = 
					vtkSmartPointer<vtkCellArray>::New();
				cells->InsertNextCell(polyLine);
 
				// Add the points to the dataset
				m_region->SetPoints(m_roiPoints);
 
				// Add the lines to the dataset
				m_region->SetLines(cells);
	
				m_region->Update();
				m_viewer->Render();
			}
		}
			
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

		//--------------
/*				// Create five points.
  double origin[3] = {0.0, 0.0, 0.0};
  double p0[3] = {10.0, 0.0, 0.0};
  double p1[3] = {0.0, 10.0, 0.0};
  double p2[3] = {0.0, 10.0, 20.0};
  double p3[3] = {10.0, 20.0, 30.0};
 
  // Create a vtkPoints object and store the points in it
  vtkSmartPointer<vtkPoints> points =
    vtkSmartPointer<vtkPoints>::New();
  points->InsertNextPoint(origin);
  points->InsertNextPoint(p0);
  points->InsertNextPoint(p1);
  points->InsertNextPoint(p2);
  points->InsertNextPoint(p3);
 
  vtkSmartPointer<vtkPolyLine> polyLine = 
    vtkSmartPointer<vtkPolyLine>::New();
  polyLine->GetPointIds()->SetNumberOfIds(5);
  for(unsigned int i = 0; i < 5; i++)
    {
    polyLine->GetPointIds()->SetId(i,i);
    }
 
  // Create a cell array to store the lines in and add the lines to it
  vtkSmartPointer<vtkCellArray> cells = 
    vtkSmartPointer<vtkCellArray>::New();
  cells->InsertNextCell(polyLine);
 
  // Create a polydata to store everything in
  vtkSmartPointer<vtkPolyData> polyData = 
    vtkSmartPointer<vtkPolyData>::New();
 
  // Add the points to the dataset
  polyData->SetPoints(points);
 
  // Add the lines to the dataset
  polyData->SetLines(cells);
 */
  // Setup actor and mapper

  m_region->SetPoints(m_roiPoints);
  vtkSmartPointer<vtkPolyDataMapper> mapper = 
    vtkSmartPointer<vtkPolyDataMapper>::New();

  mapper->SetInput(m_region);
 
  vtkSmartPointer<vtkActor> actor = 
    vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);
  
  m_viewer->GetRenderer()->AddActor(actor);

  //---------------
	}

	void MyTestInteractorStyle::setStartPoint(int* p)
	{
		m_startPoint->SetValue(p[0], p[1], p[2]);
	}

	void MyTestInteractorStyle::setEndPoint(int* p)
	{
		m_endPoint->SetValue(p[0], p[1], p[2]);
	}