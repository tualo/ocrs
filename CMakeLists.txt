cmake_minimum_required (VERSION 2.6)
project (Tutorial)
find_package( OpenCV REQUIRED )
find_package(PkgConfig)
pkg_check_modules(PC_TESSERACT QUIET tesseract)
pkg_check_modules(PC_TESSERACT QUIET zbar)

link_directories (${PC_TESSERACT_LIBS})
link_directories (${PC_ZBAR_LIBS})

add_executable(Tutorial ocrs.cpp ImageRecognize.cpp)

target_link_libraries( Tutorial ${OpenCV_LIBS} )
target_link_libraries( Tutorial tesseract )
target_link_libraries( Tutorial zbar )
