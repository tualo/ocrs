cmake_minimum_required (VERSION 2.6)
project (ocrs)
#find_package( OpenCV REQUIRED )
#find_package( MySQL REQUIRED )
find_package(PkgConfig)
pkg_check_modules(PC_OPENCV QUIET opencv3)
pkg_check_modules(PC_TESSERACT QUIET tesseract)
pkg_check_modules(PC_ZBAR_LIBS QUIET zbar)

link_directories (${PC_OPENCV_LIBS})
link_directories (${PC_TESSERACT_LIBS})
link_directories (${PC_ZBAR_LIBS})

if(WIN32)
  add_executable(ocrs ocrs.cpp ImageRecognize.cpp glob_win32.cpp glob_win32.h)
else()
  add_executable(ocrs ocrs.cpp ImageRecognize.cpp glob_posix.cpp glob_posix.h)
endif()

#target_link_libraries( ocrs ${OpenCV_LIBS} )
#target_link_libraries( ocrs opencv3 )
#-L/usr/local/Cellar/opencv3/3.0.0/lib -lopencv_calib3d -lopencv_core
#-lopencv_features2d -lopencv_flann
#-l opencv_highgui -l opencv_imgcodecs
#-l opencv_imgproc -l opencv_ml -l opencv_objdetect -l opencv_photo
#-l opencv_shape -l opencv_stitching -l opencv_superres -l opencv_video
#-lopencv_videoio -lopencv_videostab
target_link_libraries( ocrs opencv_core )
target_link_libraries( ocrs opencv_features2d )
target_link_libraries( ocrs opencv_flann )
target_link_libraries( ocrs opencv_highgui )
#target_link_libraries( ocrs opencv_imgcodecs )
target_link_libraries( ocrs opencv_imgproc )
target_link_libraries( ocrs opencv_ml )
target_link_libraries( ocrs opencv_objdetect )
target_link_libraries( ocrs opencv_photo )
#target_link_libraries( ocrs opencv_shape )
target_link_libraries( ocrs opencv_stitching )
target_link_libraries( ocrs opencv_superres )
target_link_libraries( ocrs opencv_video )
#target_link_libraries( ocrs opencv_videoio )
target_link_libraries( ocrs opencv_videostab )


target_link_libraries( ocrs tesseract )
target_link_libraries( ocrs zbar )
