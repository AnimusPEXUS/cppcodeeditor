cmake_minimum_required(VERSION 3.14)

project(unicode_test)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

# set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_DEBUG_LIBS        OFF)
set(Boost_USE_RELEASE_LIBS       ON)
set(Boost_USE_MULTITHREADED      ON)


find_package(Boost REQUIRED COMPONENTS json regex)

pkg_check_modules(gtk4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(glib REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(gtkmm4 REQUIRED IMPORTED_TARGET gtkmm-4.0)
pkg_check_modules(sigc REQUIRED IMPORTED_TARGET sigc++-3.0)

find_package(ICU 73.0 COMPONENTS uc i18n REQUIRED)

set(ICU_DEBUG ON)

add_executable(
	unicode_test 
	MWin.cpp
	main.cpp
	utils.cpp
    )

target_link_libraries(
    unicode_test 
    PUBLIC 
    # -static
    PkgConfig::gtk4 
    PkgConfig::glib
    PkgConfig::gtkmm4 
    PkgConfig::sigc 
    Boost::regex
    ICU::uc 
    ICU::i18n
    )

target_include_directories(
    unicode_test 
    PUBLIC 
    ${Boost_INCLUDE_DIRS}
    ${ICU_INCLUDE_DIRS}
    )
