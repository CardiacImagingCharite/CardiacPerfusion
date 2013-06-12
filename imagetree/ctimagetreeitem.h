/*
    Copyright 2012 Charité Universitätsmedizin Berlin, Institut für Radiologie
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

#ifndef CTIMAGETREEITEM_H
#define CTIMAGETREEITEM_H

#include <itkvtktreeitem.h>
#include <imagedefinitions.h>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "segmentationvalues.h"
#include "dicomtagtype.h"

class BinaryImageTreeItem;

/*! \class CTImageTreeItem CTImageTreeItem.h "CTImageTreeItem.h"
 *  \brief This is the representation of a CT image that can be placed in a TreeView.
 */
class CTImageTreeItem : public ITKVTKTreeItem< CTImageType >
{
  public:
    typedef ITKVTKTreeItem< CTImageType > BaseClass;
	///Constructor of the class.
    CTImageTreeItem(TreeItem * parent, DicomTagListPointer headerFields, const itk::MetaDataDictionary &_dict=itk::MetaDataDictionary(), bool IsShrinked = false, bool ResolutionChanged = false);
	
	CTImageTreeItem():m_imageTime(-1) {}
	///Clones this item. 
	/*!
	\param clonesParent The parent of the item. Default is NULL.

	\return Returns the cloned treeItem.
	*/
    virtual TreeItem *clone(TreeItem *clonesParent=NULL) const;
	///Sets data to the item. (Not implemented)
	/*!
	\param column Contains the desired column.
	\param value Contains the value that is to be set.

	\return FALSE if something went wrong. (Actually returns always FALSE)
	*/
    virtual bool setData(int column, const QVariant& value);
    virtual QVariant do_getData_DisplayRole(int column) const;
    virtual QVariant do_getData_UserRole(int column) const;
    virtual QVariant do_getData_ForegroundRole(int column) const;
    virtual Qt::ItemFlags flags(int column) const;
	///Getter for the column count.
	/*!
	\return Column count. 
	*/
    virtual int columnCount() const;
	///Getter for the unique ID.
	/*!
	\return String that contains the UID.
	*/
    virtual const std::string &getUID() const { return m_itemUID; }
	///Getter for the time.
	/*!
	\return Time.
	*/
    double getTime() const;
	///Getter for the posix time.
	/*!
	\return Posix time.
	*/
    boost::posix_time::ptime getPTime() const;
	///Getter for the segmentation values
	/*!
	\param values Placeholder for the segmentation values

	\return FALSE if something went wrong.
	*/
    bool getSegmentationValues( SegmentationValues &values) const;
    
    ///Append a filename to the list.
	/*!
	\param fn The filename.
	*/
    void appendFileName( const std::string &fn ) { m_fnList.insert( fn ); }
    ///Generates an overlay segemnt for the actual image.
	/*!
	\return A pointer to the generated Binary tree item.
	*/
    BinaryImageTreeItem *generateSegment(void);
	///Generates an overlay segemnt for the actual image with a given name.
	/*!
	\param name The name of the segemnt.
	\return A pointer to the generated Binary tree item.
	*/
    BinaryImageTreeItem *generateSegment(QString name);

	/** @name Getter for Dicom tags */
    ///@{
    static const std::string &getNumberOfFramesTag();
	static const std::string &getSeriesInstanceUIDTag();
	static const std::string &getSOPInstanceUIDTag();
	static const std::string &getAcquisitionDatetimeTag();
	static const std::string &getContentDateTag();
	static const std::string &getAquisitionTimeTag();
	///@}

    ///Get the associated ITK image.
	void retrieveITKImage(QProgressDialog *progress = NULL, int progressScale=0, int progressBase=0);
    ///Get the UID from the dicom meta data.
	static void getUIDFromDict(const itk::MetaDataDictionary &dict, std::string &iUID);
    ///Checks if a given value is a real HU value.
	/*!
	\param value The pixel value that is to be checked.

	\return TRUE or FALSE.
	*/
	static inline bool isRealHUvalue(CTPixelType value) { return (value!=-2048)?true:false; }
	///Set status of m_IsShrinked
	void setShrinked(bool shrinked=true) const;
	///Get status of m_IsShrinked
	bool isShrinked(void) const { return m_IsShrinked; }
	///Set status of m_ResolutionChanged
	void setResolutionChanged(bool changed=true) const;
	///Get status of m_ResolutionChanged
	bool isResolutionChanged(void) const { return m_ResolutionChanged; }
    
    typedef std::map< const ITKVTKTreeItem<BinaryImageType> *, SegmentationValues > SegmentationValueMap;
    ///Identifies the type of a TreeItem.
	/*!
	\param other Contains the compare type.

	\return Returns TRUE if the type matches otherwise FALSE.
	*/
	virtual bool isA(const std::type_info &other) const { 
      if (typeid(CTImageTreeItem)==other) return true;
      if (typeid(BaseClass)==other) return true;
      if (typeid(BaseClass::BaseClass)==other) return true;
      return false;
    }
    
    
  protected:
    SegmentationValueMap m_segmentationValueCache;
	///Calculate the segmentation values.
	/*!
	\param values Placeholder for the calculated values.

	\return FALSE if there are no values.
	*/
    bool internalGetSegmentationValues( SegmentationValues &values) const;
    class ReaderProgress;
    typedef std::set< std::string > FileNameList;
	///Getter for the number of slices.
	/*!
	\return Number of slices.
	*/
    int getNumberOfSlices() const;
    std::string m_itemUID;
    FileNameList m_fnList;
    DicomTagListPointer m_HeaderFields;
    itk::MetaDataDictionary m_dict;
    double m_imageTime;

  private:
    bool m_IsShrinked;
    bool m_ResolutionChanged;

    friend class boost::serialization::access;
    
    BOOST_SERIALIZATION_SPLIT_MEMBER()
    template<class Archive>
    void load(Archive & ar, const unsigned int version);
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const;
};






#endif // CTIMAGETREEITEM_H
