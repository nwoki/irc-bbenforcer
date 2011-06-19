cmake_minimum_required(VERSION 2.8)
project (ioQIC-bbenforcer)

# Include our extra FindFoo.cmake files.
set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)

set(QT_MIN_VERSION "4.7.3")
FIND_PACKAGE(Qt4 ${QT_MIN_VERSION} REQUIRED)

# load modules needed for the project. "include" sets up the ${QT_LIBRARIES} var
set(QT_USE_QTNETWORK TRUE)
set(QT_USE_QTSQL TRUE)
set(QT_USE_QTXML TRUE)

include(${QT_USE_FILE})

set(ioQIC_SRCS
    src/brain.cpp
    src/dbcontroller.cpp
    src/gamecontroller.cpp
    src/irccontroller.cpp
    src/ircuserscontainer.cpp
    src/logger.cpp
    src/main.cpp
)

#headers that need to use MOC (inheriting QObject)
set(ioQIC_HEADERS
    src/brain.h
    src/gamecontroller.h
    src/irccontroller.h
    src/ircuserscontainer.h
)

QT4_WRAP_CPP(ioQIC_HEADERS_MOC ${ioQIC_HEADERS})


add_executable(ioQIC-bbenforcer ${ioQIC_SRCS} ${ioQIC_HEADERS_MOC})

target_link_libraries(ioQIC-bbenforcer ${QT_LIBRARIES})
