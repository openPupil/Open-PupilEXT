
if (MSVC)
    add_definitions(-DNOMINMAX)
endif()

set(RESOURCE "${PROJECT_SOURCE_DIR}/PupilExt.qrc")
qt5_add_resources(RESOURCE_ADDED ${RESOURCE})


set(APP_ICON_RESOURCE_WINDOWS "${PROJECT_SOURCE_DIR}/PupilExt.rc")

set(MACOSX_BUNDLE_ICON_FILE icon.icns)
set(APP_ICON_MACOSX ${PROJECT_SOURCE_DIR}/icon.icns)
set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")


# For packaging add the following code - add_executable(PupilEXT MACOSX_BUNDLE main.cpp
add_executable(PupilEXT main.cpp ${RESOURCE_ADDED} ${APP_ICON_RESOURCE_WINDOWS} ${APP_ICON_MACOSX}
        mainwindow.cpp
        supportFunctions.h
        subwindows/serialSettingsDialog.cpp subwindows/serialSettingsDialog.h
        dataWriter.cpp dataWriter.h
        signalPubSubHandler.h
        subwindows/gettingsStartedWizard.cpp subwindows/gettingsStartedWizard.h
        pupil-detection-methods/Pupil.h pupil-detection-methods/PupilDetectionMethod.h
        pupil-detection-methods/ElSe.cpp pupil-detection-methods/ElSe.h
        pupil-detection-methods/ExCuSe.cpp pupil-detection-methods/ExCuSe.h
        pupil-detection-methods/PuRe.cpp pupil-detection-methods/PuRe.h
        pupil-detection-methods/Swirski2D.cpp pupil-detection-methods/Swirski2D.h
        pupil-detection-methods/Starburst.cpp pupil-detection-methods/Starburst.h
        pupil-detection-methods/PupilDetectionMethod.cpp
        pupil-detection-methods/PuReST.cpp pupil-detection-methods/PuReST.h
        pupil-detection-methods/Swirski3D.cpp pupil-detection-methods/Swirski3D.h
        subwindows/qcustomplot/qcustomplot.cpp subwindows/qcustomplot/qcustomplot.h
        subwindows/graphPlot.cpp subwindows/graphPlot.h
        subwindows/dataTable.cpp subwindows/dataTable.h
        subwindows/singleCameraView.cpp subwindows/singleCameraView.h
        subwindows/stereoCameraView.cpp subwindows/stereoCameraView.h
        subwindows/singleCameraSettingsDialog.cpp subwindows/singleCameraSettingsDialog.h
        subwindows/videoView.cpp subwindows/videoView.h
        subwindows/imageGraphicsItem.h
        devices/camera.h
        devices/singleCamera.cpp devices/singleCamera.h
        devices/singleCameraImageEventHandler.cpp devices/singleCameraImageEventHandler.h
        devices/hardwareTriggerConfiguration.h
        frameRateCounter.h
        subwindows/singleCameraCalibrationView.cpp subwindows/singleCameraCalibrationView.h
        cameraCalibration.cpp cameraCalibration.h
        devices/stereoCamera.h devices/stereoCamera.cpp
        subwindows/pupilDetectionSettingsDialog.h subwindows/pupilDetectionSettingsDialog.cpp
        pupilDetection.cpp pupilDetection.h
        subwindows/pupil-detection-methods/PupilMethodSetting.h
        subwindows/pupil-detection-methods/PuReSettings.h subwindows/pupil-detection-methods/ElSeSettings.h
        subwindows/pupil-detection-methods/ExCuSeSettings.h subwindows/pupil-detection-methods/StarburstSettings.h subwindows/pupil-detection-methods/Swirski2DSettings.h
        subwindows/pupil-detection-methods/PuReSTSettings.h imageWriter.cpp imageWriter.h imageReader.cpp imageReader.h
        devices/fileCamera.h devices/fileCamera.cpp subwindows/ResizableRectItem.cpp subwindows/ResizableRectItem.h
        subwindows/stereoCameraSettingsDialog.cpp subwindows/stereoCameraSettingsDialog.h
        devices/stereoCameraImageEventHandler.cpp devices/stereoCameraImageEventHandler.h
        subwindows/stereoCameraCalibrationView.h subwindows/stereoCameraCalibrationView.cpp
        stereoCameraCalibration.h stereoCameraCalibration.cpp cameraFrameRateCounter.h subwindows/generalSettingsDialog.cpp subwindows/generalSettingsDialog.h subwindows/stereoFileCameraCalibrationView.cpp subwindows/stereoFileCameraCalibrationView.h
        execArgParser.h execArgParser.cpp eyeDataSerializer.h eyeDataSerializer.cpp camTempMonitor.h camTempMonitor.cpp dataStreamer.h dataStreamer.cpp metaSnapshotOrganizer.h metaSnapshotOrganizer.cpp
        devices/singleWebcam.h devices/singleWebcam.cpp devices/singleWebcamImageEventHandler.h devices/singleWebcamImageEventHandler.cpp
        subwindows/singleWebcamSettingsDialog.h subwindows/singleWebcamSettingsDialog.cpp
        connPoolCOM.h PRGmainwindow.cpp
        recEventTracker.h recEventTracker.cpp
        subwindows/streamingSettingsDialog.h subwindows/streamingSettingsDialog.cpp
        subwindows/singleWebcamCalibrationView.h subwindows/singleWebcamCalibrationView.cpp
        subwindows/IPCtrl.h subwindows/IPCtrl.cpp
        subwindows/remoteCCDialog.h subwindows/remoteCCDialog.cpp
        subwindows/playbackDial.h subwindows/playbackSlider.h subwindows/imagePlaybackControlDialog.h subwindows/imagePlaybackControlDialog.cpp
        subwindows/singleFileCameraCalibrationView.h subwindows/singleFileCameraCalibrationView.cpp subwindows/RestorableQMdiSubWindow.h subwindows/subjectSelectionDialog.cpp subwindows/subjectSelectionDialog.h
        subwindows/singleCameraSharpnessView.h subwindows/singleCameraSharpnessView.cpp sharpnessCalculation.h sharpnessCalculation.cpp
        subwindows/calibrationHelpDialog.h subwindows/calibrationHelpDialog.cpp
        subwindows/pupil-detection-methods/json.h subwindows/timestampSpinBox.h subwindows/timestampSpinBox.cpp
        SVGIconColorAdjuster.h
        camImageRegionsWidget.h camImageRegionsWidget.cpp
        playbackSynchroniser.cpp
        playbackSynchroniser.h
        subwindows/outputDataRuleDialog.h
        subwindows/outputDataRuleDialog.cpp
        devices/cameraConfigurationEventHandler.h
)

# On linux with GCC and QT 5.15.2 setting standard here is recognized and is not overwritten by QT
#set_property(TARGET PupilEXT PROPERTY CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD 17)

if(SPII_BUILD_FOUND)

    target_link_libraries(${CMAKE_PROJECT_NAME}
            "singleeyefitter"
            Qt5::Widgets Qt5::Concurrent Qt5::SerialPort Qt5::Charts Qt5::Svg Qt5::PrintSupport Qt5::Network Qt5::Xml #Qt5::OpenGL
            ${Boost_LIBRARIES}
            TBB::tbb
            #Boost::boost
            ${spii_LIBRARIES}
            ${CERES_LIBRARIES}
            ${PYLON_LIBRARIES}
            ${OpenCV_LIBS}
            #opengl32
            )

else()

    target_link_libraries(${CMAKE_PROJECT_NAME}
        "singleeyefitter"
        Qt5::Widgets Qt5::Concurrent Qt5::SerialPort Qt5::Charts Qt5::Svg Qt5::PrintSupport Qt5::Network Qt5::Xml #Qt5::OpenGL
        ${Boost_LIBRARIES}
        TBB::tbb
        #Boost::boost
        spii
        meschach
        ${CERES_LIBRARIES}
        ${PYLON_LIBRARIES}
        ${OpenCV_LIBS}
        #opengl32
    )

endif()

# add_definitions(-DQCUSTOMPLOT_USE_OPENGL)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


if(MSVC OR WIN32)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W3 /MP)
else()
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -pedantic)
endif()

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION "${PROJECT_SOURCE_DIR}/bin/debug" CONFIGURATIONS Debug)
install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION "${PROJECT_SOURCE_DIR}/bin/release" CONFIGURATIONS Release)

install(PROGRAMS ${__location_release} DESTINATION "${PROJECT_SOURCE_DIR}/bin/release")

if(MSVC OR WIN32)
    install(PROGRAMS "$<TARGET_FILE:Qt5::QWindowsIntegrationPlugin>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release/platforms")
    install(PROGRAMS "$<TARGET_FILE:Qt5::QWindowsVistaStylePlugin>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release/styles")
endif()

install(PROGRAMS "$<TARGET_FILE:Qt5::QSvgIconPlugin>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release/iconengines")
install(PROGRAMS "$<TARGET_FILE:Qt5::QSvgPlugin>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release/imageformats")
install(PROGRAMS "$<TARGET_FILE:Qt5::PrintSupport>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release/printsupport")
install(PROGRAMS "$<TARGET_FILE:Qt5::Network>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release/network")
install(PROGRAMS "$<TARGET_FILE:Qt5::Xml>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release/xml")

install(PROGRAMS "$<TARGET_FILE:Qt5::Gui>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release")
install(PROGRAMS "$<TARGET_FILE:Qt5::Core>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release")
install(PROGRAMS "$<TARGET_FILE:Qt5::Widgets>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release")
install(PROGRAMS "$<TARGET_FILE:Qt5::SerialPort>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release")
install(PROGRAMS "$<TARGET_FILE:Qt5::Charts>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release")
install(PROGRAMS "$<TARGET_FILE:Qt5::Svg>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release")
##install(PROGRAMS "$<TARGET_FILE:Qt5::OpenGL>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release")

install(PROGRAMS "$<TARGET_FILE:TBB::tbb>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release")

if(MSVC OR WIN32)
    #install(PROGRAMS "$<TARGET_FILE:opencv_world>" DESTINATION "${PROJECT_SOURCE_DIR}/bin/release")
endif()

if(MSVC OR WIN32)
    install(PROGRAMS "$<TARGET_FILE:Qt5::QWindowsIntegrationPlugin>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/platforms")
    install(PROGRAMS "$<TARGET_FILE:Qt5::QWindowsVistaStylePlugin>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/styles")
endif()

install(PROGRAMS "$<TARGET_FILE:Qt5::QSvgIconPlugin>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/iconengines")
install(PROGRAMS "$<TARGET_FILE:Qt5::QSvgPlugin>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/imageformats")
install(PROGRAMS "$<TARGET_FILE:Qt5::PrintSupport>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/printsupport")
install(PROGRAMS "$<TARGET_FILE:Qt5::Network>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/network")
install(PROGRAMS "$<TARGET_FILE:Qt5::Xml>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/xml")

install(PROGRAMS "$<TARGET_FILE:Qt5::Gui>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
install(PROGRAMS "$<TARGET_FILE:Qt5::Core>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
install(PROGRAMS "$<TARGET_FILE:Qt5::Widgets>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
install(PROGRAMS "$<TARGET_FILE:Qt5::SerialPort>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
install(PROGRAMS "$<TARGET_FILE:Qt5::Charts>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
install(PROGRAMS "$<TARGET_FILE:Qt5::Svg>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
##install(PROGRAMS "$<TARGET_FILE:Qt5::OpenGL>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

install(PROGRAMS "$<TARGET_FILE:TBB::tbb>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
#install(PROGRAMS "$<TARGET_FILE:opencv_world>" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

