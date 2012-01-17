PROJECT(SimpleView)

IF(NOT VTK_BINARY_DIR)
FIND_PACKAGE(VTK)
IF(NOT VTK_DIR)
  MESSAGE(FATAL_ERROR "Please set VTK_DIR.")
ENDIF(NOT VTK_DIR)
INCLUDE(${VTK_USE_FILE})
ENDIF(NOT VTK_BINARY_DIR)

FIND_PACKAGE ( ITK )
IF ( ITK_FOUND )
      INCLUDE( ${ITK_USE_FILE} )
ELSE(ITK_FOUND)
  MESSAGE(FATAL_ERROR "Cannot build without ITK.  Please set ITK_DIR.")
ENDIF( ITK_FOUND )

FIND_PACKAGE ( GDCM REQUIRED )
INCLUDE( ${GDCM_USE_FILE} )

IF(WIN32)
  SET(GDCM_LIBRARY_DIRS "${GDCM_DIR}/bin")
  SET(GDCM_LIBRARIES gdcmcharls.lib gdcmCommon.lib gdcmDICT.lib
    gdcmDSED.lib gdcmexpat.lib gdcmgetopt.lib gdcmIOD.lib gdcmjpeg8.lib 
	gdcmjpeg12.lib gdcmjpeg16.lib gdcmMSFF.lib gdcmopenjpeg.lib gdcmzlib.lib)
ELSE(WIN32)
ENDIF(WIN32)

# use what QVTK built with
SET(QT_MOC_EXECUTABLE ${VTK_QT_MOC_EXECUTABLE} CACHE FILEPATH "")
SET(QT_UIC_EXECUTABLE ${VTK_QT_UIC_EXECUTABLE} CACHE FILEPATH "")
SET(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE} CACHE FILEPATH "")
FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})


# Use the include path and library for Qt that is used by VTK.
INCLUDE_DIRECTORIES(
  ${QT_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${ITK_SOURCE_DIR}/../InsightApplications-3.20.0/Auxiliary/vtk
)

LINK_DIRECTORIES(
 ${GDCM_LIBRARY_DIRS}
 ${ITK_SOURCE_DIR}/../binITKApplications/VolviewPlugIns/bin/Debug
)


# Set your files and resources here  
SET(SimpleViewSrcs main.cxx SimpleView.cxx)
SET(SimpleViewUI SimpleView.ui)
SET(SimpleViewHeaders SimpleView.h)
SET(SimpleViewResources Icons/icons.qrc)

# The rest should just work (sure...)
QT4_WRAP_UI(UISrcs ${SimpleViewUI})
QT4_WRAP_CPP(MOCSrcs ${SimpleViewHeaders} )
QT4_ADD_RESOURCES(ResourceSrcs ${SimpleViewResources})

SOURCE_GROUP("Resources" FILES
  ${SimpleViewUI}
  ${SimpleViewResources}
  ${EXE_ICON}
)

SOURCE_GROUP("Generated" FILES
  ${UISrcs}
  ${MOCSrcs}
  ${ResourceSrcs}
  ${RCS_SOURCES}
)

ADD_DEFINITIONS(-DQT_GUI_LIBS -DQT_CORE_LIB -DQT3_SUPPORT)
SET_SOURCE_FILES_PROPERTIES(${SimpleViewSrcs} PROPERTIES
                            OBJECT_DEPENDS "${UISrcs}")

# It's nice to have the ui in the windows project file...just double click on it
# and designer comes up on that ui file :)
IF (${CMAKE_BUILD_TOOL} MATCHES "msdev")
  SET (SimpleViewSrcs ${SimpleViewSrcs} ${SimpleViewUI})
ENDIF (${CMAKE_BUILD_TOOL} MATCHES "msdev")
IF (${CMAKE_BUILD_TOOL} MATCHES "devenv")
  SET (SimpleViewSrcs ${SimpleViewSrcs} ${SimpleViewUI})
ENDIF (${CMAKE_BUILD_TOOL} MATCHES "devenv")

ADD_EXECUTABLE( SimpleView MACOSX_BUNDLE ${SimpleViewSrcs} ${UISrcs} ${MOCSrcs} ${ResourceSrcs})

TARGET_LINK_LIBRARIES( SimpleView
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
  vtkHybrid
  vtkImaging 
  vtkFiltering
  vtkVolumeRendering
  ITKVTKIntegration
)



