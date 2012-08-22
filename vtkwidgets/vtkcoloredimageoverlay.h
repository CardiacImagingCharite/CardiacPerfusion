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
			  int &actionHandle, double opacity = 0.3);
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