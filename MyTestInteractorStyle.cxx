// Define interaction style

#include "MyTestInteractorStyle.h"
#include "vtkRenderWindowInteractor.h"
 
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
	
	}

	void MyTestInteractorStyle::OnLeftButtonUp()
	{
	
	}

	void MyTestInteractorStyle::setImageViewer(vtkImageViewer2 * viewer)
	{
		this->m_viewer = viewer;
	}
