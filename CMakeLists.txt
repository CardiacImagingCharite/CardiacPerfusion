cmake_minimum_required(VERSION 2.8)

PROJECT(KardioPerfusion)


# Set your files and resources here  

SET( KardioPerfusionSrcs 
	main/main.cxx 
	main/KardioPerfusion.cxx
	main/KardioPerfusion.h
	main/dicomselectordialog.cpp
	main/dicomselectordialog.h	
	main/MyTestInteractorStyle.cxx
	main/MyTestInteractorStyle.h

	imagetree/ctimagetreemodel.cpp
	imagetree/dicomtagtype.cpp
	imagetree/vtkconnectordatabase.cpp
	imagetree/treeitem.cpp
	imagetree/ctimagetreeitem.cpp 
	imagetree/binaryimagetreeitem.cpp 
	imagetree/itkvtktreeitem.cpp 
	imagetree/segmentationvalues.cpp

#	serialization/ctimagetreemodel_serializer.cpp 
)

SET( KardioPerfusionUI 
	main/KardioPerfusion.ui
	main/DicomSelectDialog.ui
)

SET( KardioPerfusionHeaders
	main/KardioPerfusion.h
	main/dicomselectordialog.h

	imagetree/ctimagetreemodel.h
)

SET( KardioPerfusionResources 
	Icons/icons.qrc
)

SET( HeaderDirs
	main
#	analyse
	imagetree
#	serialization
	misc
#	vtkwidgets
)


#Find VTK directory
IF(NOT VTK_BINARY_DIR)
FIND_PACKAGE(VTK)
IF(NOT VTK_DIR)
  MESSAGE(FATAL_ERROR "Please set VTK_DIR.")
ENDIF(NOT VTK_DIR)
INCLUDE(${VTK_USE_FILE})
ENDIF(NOT VTK_BINARY_DIR)

#Find ITK directory 
FIND_PACKAGE ( ITK )
IF ( ITK_FOUND )
      INCLUDE( ${ITK_USE_FILE} )
ELSE(ITK_FOUND)
  MESSAGE(FATAL_ERROR "Cannot build without ITK.  Please set ITK_DIR.")
ENDIF( ITK_FOUND )

#Find GDCM directory
FIND_PACKAGE ( GDCM REQUIRED )
INCLUDE( ${GDCM_USE_FILE} )

IF(WIN32)
  SET(GDCM_LIBRARY_DIRS "${GDCM_DIR}/bin")
  SET(GDCM_LIBRARIES gdcmcharls.lib gdcmCommon.lib gdcmDICT.lib
    gdcmDSED.lib gdcmexpat.lib gdcmgetopt.lib gdcmIOD.lib gdcmjpeg8.lib 
	gdcmjpeg12.lib gdcmjpeg16.lib gdcmMSFF.lib gdcmopenjpeg.lib gdcmzlib.lib)
ELSE(WIN32)
ENDIF(WIN32)

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

FIND_PATH(ITK_APPS_LIB VolviewPlugIns
		HINTS ITK_DIR/../)


# Use the include path and library for Qt that is used by VTK and for the ITK-VTK-Filter
INCLUDE_DIRECTORIES(
  ${QT_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  
  ${ITK_APPS}/Auxiliary/vtk
  ${HeaderDirs}
)

LINK_DIRECTORIES(
 ${Boost_LIBRARY_DIRS}
 ${GDCM_LIBRARY_DIRS}
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
  ITKBasicFilters ITKCommon ITKAlgorithms ITKIO
  vtkRendering
  vtkGraphics
  vtkIO
  vtkCommon
  vtkInfovis
  vtkViews
  ${GDCM_LIBRARIES}
  ${Boost_LIBRARIES}
  vtkHybrid
  vtkImaging 
  vtkFiltering
  vtkVolumeRendering
  ITKVTKIntegration
)



