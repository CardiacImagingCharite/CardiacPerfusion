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

#ifndef ITKVTKTREEITEM_H
#define ITKVTKTREEITEM_H

#include "treeitem.h"
#include "ctimagetreemodel.h"
#include <itkImageToVTKImageFilter.h>
#include <itkCommand.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <boost/scoped_ptr.hpp>
#include <QProgressDialog>
#include <QObject>
#include <set>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "vtkconnectordatabase.h"

template< class TImage >
/*! \class ITKVTKTreeItem ITKVTKTreeItem.h "ITKVTKTreeItem.h"
 *  \brief This class represents a tree item for ITK and VTK items.
 */
class ITKVTKTreeItem : public TreeItem {
	public:
    typedef TreeItem BaseClass;
    typedef ITKVTKTreeItem< TImage > Self;
    typedef TImage ImageType;
    typedef typename ImageType::Pointer ImagePointerType;
    typedef itk::ImageToVTKImageFilter< ImageType > ConnectorType;
    
    class ConnectorData : public VTKConnectorDataBase {
      public:
	virtual vtkImageData *getVTKImageData() const {
	  if (connector.IsNotNull()) return connector->GetOutput();
	  return NULL;
	}
	virtual size_t getSize() const {
	  if (itkImage) {
	    typename ImageType::RegionType reg = itkImage->GetBufferedRegion();
	    return reg.GetSize(0) * reg.GetSize(1) * reg.GetSize(2) * sizeof(typename ImageType::PixelType);
	  }
	  return 0;
	}
	ConnectorData(
	  ITKVTKTreeItem<TImage> *baseItem_, 
	  typename ImageType::Pointer itkImage_)
	    :VTKConnectorDataBase(baseItem_), connector(ConnectorType::New()) {
	      setITKImage( itkImage_ );
	    }
	typename ConnectorType::Pointer getConnector() const { return connector; }
	typename ImageType::Pointer getITKImage() const { return itkImage; }
	void setITKImage(typename ImageType::Pointer i) { 
	  itkImage = i;
	  connector->SetInput(i);
	  connector->Update();
	}
      private:
      typename ImageType::Pointer itkImage;
      typename ConnectorType::Pointer connector;
    };
    typedef VTKConnectorDataBasePtr ConnectorHandle;
   
    ITKVTKTreeItem(TreeItem * parent, ImagePointerType itkI = ImagePointerType())
      :TreeItem(parent) { setITKImage( itkI ); }
    typename ImageType::Pointer getITKImage(QProgressDialog *progress = NULL, int progressScale=0, int progressBase=0) const;
    virtual void retrieveITKImage(QProgressDialog *progress = NULL, int progressScale=0, int progressBase=0) {}
    virtual ConnectorHandle getVTKConnector(QProgressDialog *progress = NULL, int progressScale=0, int progressBase=0) const {
      ConnectorHandle connData( weakConnector.lock());
      if (!connData) {
	const_cast<ITKVTKTreeItem<TImage>*>(this)->retrieveITKImage(progress, progressScale, progressBase);
	connData = weakConnector.lock();
      } else { 	model->registerConnectorData(connData); }
//      dynamic_cast<ConnectorData*>(connData.get())->getConnector()->Update();
      return connData;
    }
    unsigned long getITKMTime(void) const {
      typename ImageType::Pointer fullPtr = peekITKImage();
      if (fullPtr.IsNotNull()) return fullPtr->GetMTime(); else return 0;
    }
    ~ITKVTKTreeItem() { }
    virtual bool isA(const std::type_info &other) const { 
      if (typeid(Self)==other) return true;
      if (typeid(BaseClass)==other) return true;
      return false;
    }
    
  protected:
    inline typename ImageType::Pointer peekITKImage(void) const { 
      ConnectorHandle tHand = weakConnector.lock();
      if (tHand) {
	return dynamic_cast<ConnectorData*>(tHand.get())->getITKImage();
      }
      return typename ImageType::Pointer();
    }
    inline void setITKImage(typename ImageType::Pointer image) { 
      if (image) {
	ConnectorHandle connData( weakConnector.lock() );
	if (!connData) {
	  connData.reset(new ConnectorData(this, image));
	  weakConnector = connData;
	} else {
	  ConnectorData *conn =  dynamic_cast<ConnectorData*>(connData.get());
	  conn->setITKImage(image);
	}
	model->registerConnectorData(connData);
      }
    }
  protected:

  private:
    typedef boost::weak_ptr<VTKConnectorDataBasePtr::value_type> WeakConnectorHandle;
    WeakConnectorHandle weakConnector;
    
  protected:
    ITKVTKTreeItem() {}

  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};




template< class TImage >
typename ITKVTKTreeItem<TImage>::ImageType::Pointer ITKVTKTreeItem<TImage>::getITKImage(QProgressDialog *progress, int progressScale, int progressBase) const {
  typename ImageType::Pointer iptr = peekITKImage();
  if (iptr.IsNull()) const_cast<ITKVTKTreeItem<TImage>*>(this)->retrieveITKImage();
  return peekITKImage();
}


#endif // ITKVTKTREEITEM_H
