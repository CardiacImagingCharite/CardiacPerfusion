
#####################################################################################
#										    #
#    Copyright 2012 Charité Universitätsmedizin Berlin, Institut für Radiologie	    #
#                                                                                   #
#	 This file is part of KardioPerfusion.                                      #
#                                                                                   #
#    KardioPerfusion is free software: you can redistribute it and/or modify        #
#    it under the terms of the GNU General Public License as published by           #
#    the Free Software Foundation, either version 3 of the License, or              #
#    (at your option) any later version.                                            #
#                                                                                   #
#    KardioPerfusion is distributed in the hope that it will be useful,             #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of                 #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                  #
#    GNU General Public License for more details.                                   #
#                                                                                   #
#    You should have received a copy of the GNU General Public License              #
#    along with KardioPerfusion.  If not, see <http://www.gnu.org/licenses/>.       #
#                                                                                   #
#    Diese Datei ist Teil von KardioPerfusion.                                      #
#                                                                                   #
#    KardioPerfusion ist Freie Software: Sie können es unter den Bedingungen        #
#    der GNU General Public License, wie von der Free Software Foundation,          #
#    Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren                   #
#    veröffentlichten Version, weiterverbreiten und/oder modifizieren.              #
#                                                                                   #
#    KardioPerfusion wird in der Hoffnung, dass es nützlich sein wird, aber         #
#    OHNE JEDE GEWÄHRLEISTUNG, bereitgestellt; sogar ohne die implizite             #
#    Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.     #
#    Siehe die GNU General Public License für weitere Details.                      #
#                                                                                   #
#    Sie sollten eine Kopie der GNU General Public License zusammen mit diesem      #
#    Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.     #
#                                                                                   #
#                                                                                   #
#####################################################################################


cmake_minimum_required(VERSION 2.8)

PROJECT(KardioPerfusion)

IF(WIN32)
  IF (NOT CMAKE_CXX_FLAGS MATCHES "/MP")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") 
  ENDIF ()
ENDIF(WIN32)

IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x -O3 -fpermissive")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -stdlib=libc++")
ENDIF()

# Set your files and resources here  

SET( KardioPerfusionSrcs 
	main/main.cxx 
	main/KardioPerfusion.cxx
	main/KardioPerfusion.h
	main/dicomselectordialog.cpp
	main/dicomselectordialog.h
	main/perfusionMapCreator.cxx
	main/perfusionMapCreator.h
#	main/MyTestInteractorStyle.cxx
#	main/MyTestInteractorStyle.h

	analyse/perfusionAnalyzer.cxx
	analyse/perfusionAnalyzer.h
	analyse/mmid4Analyzer.cxx
	analyse/mmid4Analyzer.h
	analyse/maxSlopeAnalyzer.cxx
	analyse/maxSlopeAnalyzer.h
	analyse/tacdialog.cpp
	analyse/tacdialog.h
	analyse/timedensitydata.cpp
	analyse/timedensitydata.h	
#	analyse/timedensitydatapicker.cpp
#	analyse/timedensitydatapicker.h
	analyse/segmentlistmodel.cpp
	analyse/segmentlistmodel.h
	analyse/segmentinfo.cpp
	analyse/segmentinfo.h
	analyse/linearregressiondata.cpp
	analyse/linearregressiondata.h
	analyse/linearregression.h
	analyse/gammafitdata.cpp
	analyse/gammafitdata.h
	analyse/gammaVariate.cpp
	analyse/gammaVariate.h
	analyse/patlakdata.cpp
	analyse/patlakdata.h
#	analyse/SamplePoint.cxx
#	analyse/SamplePoint.h

	serialization/ctimagetreemodel_serializer.cpp 
	serialization/ctimagetreemodel_serializer.h

	imagetree/ctimagetreemodel.cpp
	imagetree/ctimagetreemodel.h
	imagetree/dicomtagtype.cpp
	imagetree/dicomtagtype.h
	imagetree/vtkconnectordatabase.cpp
	imagetree/vtkconnectordatabase.h
	imagetree/treeitem.cpp
	imagetree/treeitem.h
	imagetree/ctimagetreeitem.cpp 
	imagetree/ctimagetreeitem.h
	imagetree/binaryimagetreeitem.cpp
	imagetree/binaryimagetreeitem.h 
	imagetree/itkvtktreeitem.cpp  
	imagetree/itkvtktreeitem.h
	imagetree/segmentationvalues.cpp
	imagetree/segmentationvalues.h
	imagetree/realimagetreeitem.cxx
	imagetree/realimagetreeitem.h

	vtkwidgets/vtkbinaryimagetocolor.cpp 
	vtkwidgets/vtkbinaryimagetocolor.h
	vtkwidgets/vtkbinaryimageoverlay.cpp
	vtkwidgets/vtkbinaryimageoverlay.h 
	vtkwidgets/vtkcoloredimageoverlay.h
	vtkwidgets/vtkcoloredimageoverlay.cxx
#	vtkwidgets/interactorstylevolumeview.cpp 
	vtkwidgets/multiplanarreformatwidget.cpp 
	vtkwidgets/multiplanarreformatwidget.h
	vtkwidgets/vtkinteractorstyleprojectionview.cpp 
	vtkwidgets/vtkinteractorstyleprojectionview.h
#	vtkwidgets/volumeprojectionwidget.cpp 
	vtkwidgets/vtkinteractoractiondispatch.h
#	serialization/ctimagetreemodel_serializer.cpp 

	qtwidgets/mytabwidget.h
	qtwidgets/mytabwidget.cxx
	qtwidgets/segmentselectbutton.cpp
	qtwidgets/segmentselectbutton.h

	itkfilter/itkperfusionmapfilter.txx
	itkfilter/itkperfusionmapfilter.h

	itkfilter/itkNaryPerfusionImageFilter.h
	itkfilter/autoAlignHeart.cxx
	itkfilter/autoAlignHeart.h
	itkfilter/itkFindLeftVentricle.txx
	itkfilter/itkFindLeftVentricle.h
	itkfilter/itkShrinkAverageFilter.txx
	itkfilter/itkShrinkAverageFilter.h
	itkfilter/itkBinShrinkImageFilter.h
	itkfilter/itkBinShrinkImageFilter.hxx

)

SET( KardioPerfusionUI 
	main/KardioPerfusion.ui
	main/DicomSelectDialog.ui
	analyse/TacDialog.ui
)

SET( KardioPerfusionHeaders
	main/KardioPerfusion.h
	main/dicomselectordialog.h
	vtkwidgets/multiplanarreformatwidget.h
	analyse/tacdialog.h
	imagetree/ctimagetreemodel.h
	qtwidgets/mytabwidget.h
	qtwidgets/segmentselectbutton.h
	vtkwidgets/vtkinteractorstyleprojectionview.h
)

SET( KardioPerfusionResources 
	Icons/icons.qrc
)

SET( HeaderDirs
	main
	analyse
	imagetree
	serialization
	misc
	vtkwidgets
	qtwidgets
	itkfilter
)

FIND_PATH(QWT_INCLUDE_DIR qwt.h)
FIND_PATH(QWT_LIBRARY_DIR libqwt.so)
IF(WIN32)
  SET(QWT_LIBRARIES qwt)
ELSE(WIN32)
  SET(QWT_LIBRARIES qwt)
ENDIF(WIN32)


#Find VTK directory
IF(NOT VTK_BINARY_DIR)
FIND_PACKAGE(VTK)
IF(NOT VTK_DIR)
  MESSAGE(FATAL_ERROR "Please set VTK_DIR.")
ENDIF(NOT VTK_DIR)
INCLUDE(${VTK_USE_FILE})
ENDIF(NOT VTK_BINARY_DIR)

#Find ITK directory 
FIND_PACKAGE ( ITK 4.4 REQUIRED )
IF ( ITK_FOUND )
      INCLUDE( ${ITK_USE_FILE} )
ELSE(ITK_FOUND)
  MESSAGE(FATAL_ERROR "Cannot build without ITK.  Please set ITK_DIR.")
ENDIF( ITK_FOUND )

SET(Boost_USE_STATIC_LIBS   ON)
SET(Boost_USE_MULTITHREADED ON)
FIND_PACKAGE(Boost 1.40.0 REQUIRED COMPONENTS filesystem signals serialization iostreams date_time system)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

# use what QVTK built with
SET(QT_MOC_EXECUTABLE ${VTK_QT_MOC_EXECUTABLE} CACHE FILEPATH "")
SET(QT_UIC_EXECUTABLE ${VTK_QT_UIC_EXECUTABLE} CACHE FILEPATH "")
SET(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE} CACHE FILEPATH "")
FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

FIND_PATH(ITK_APPS FindInsightApplications.cmake
		HINTS ITK_DIR/../)


# Use the include path and library for Qt that is used by VTK and for the ITK-VTK-Filter
INCLUDE_DIRECTORIES(
  ${QT_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${QWT_INCLUDE_DIR}
  ${ITK_APPS}/Auxiliary/vtk
  ${HeaderDirs}
)

LINK_DIRECTORIES(
 ${Boost_LIBRARY_DIRS}
 ${GDCM_LIBRARY_DIRS}
 ${QWT_LIBRARY_DIR}
 ${ITK_APPS_LIB}/VolviewPlugIns/bin/Debug
)


# The rest should just work (sure...)
QT4_WRAP_UI(UISrcs ${KardioPerfusionUI})
QT4_WRAP_CPP(MOCSrcs ${KardioPerfusionHeaders} OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED )
QT4_ADD_RESOURCES(ResourceSrcs ${KardioPerfusionResources})

SOURCE_GROUP("Resources" FILES
  ${KardioPerfusionUI}
  ${KardioPerfusionResources}
  ${EXE_ICON}
)

SOURCE_GROUP("Generated" FILES
  ${UISrcs}
  ${MOCSrcs}
  ${ResourceSrcs}
  ${RCS_SOURCES}
)

ADD_DEFINITIONS(-DQT_GUI_LIBS -DQT_CORE_LIB -DQT3_SUPPORT)
SET_SOURCE_FILES_PROPERTIES(${KardioPerfusionSrcs} PROPERTIES
                            OBJECT_DEPENDS "${UISrcs}")

# It's nice to have the ui in the windows project file...just double click on it
# and designer comes up on that ui file :)
IF (${CMAKE_BUILD_TOOL} MATCHES "msdev")
  SET (KardioPerfusionSrcs ${KardioPerfusionSrcs} ${KardioPerfusionUI})
ENDIF (${CMAKE_BUILD_TOOL} MATCHES "msdev")
IF (${CMAKE_BUILD_TOOL} MATCHES "devenv")
  SET (KardioPerfusionSrcs ${KardioPerfusionSrcs} ${KardioPerfusionUI})
ENDIF (${CMAKE_BUILD_TOOL} MATCHES "devenv")

ADD_EXECUTABLE( KardioPerfusion MACOSX_BUNDLE ${KardioPerfusionSrcs} ${UISrcs} ${MOCSrcs} ${ResourceSrcs})
#ADD_EXECUTABLE( KardioPerfusion ${KardioPerfusionSrcs} ${MOCSrcs})

TARGET_LINK_LIBRARIES( KardioPerfusion
  QVTK
  ${QT_LIBRARIES}
#  ITKBasicFilters ITKCommon ITKAlgorithms ITKIO
  ${ITK_LIBRARIES}
  vtkRendering
  vtkGraphics
  vtkIO
  vtkCommon
  vtkInfovis
  vtkViews
  ${GDCM_LIBRARIES}
  ${Boost_LIBRARIES}
  ${QWT_LIBRARIES}
  vtkHybrid
  vtkImaging 
  vtkFiltering
  vtkVolumeRendering
#  ITKVTKIntegration
z
)

# add a target to generate API documentation with Doxygen

FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  ADD_CUSTOM_TARGET(DOCUMENTATION ALL
  ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)

