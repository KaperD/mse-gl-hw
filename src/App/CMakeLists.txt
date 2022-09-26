set(SRCS
    main.cpp
        FractalWindow.cpp
        FractalWindow.h

    shaders.qrc
    Shaders/diffuse.fs
    Shaders/diffuse.vs
        Slider.h Slider.cpp)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(demo-app ${SRCS})

target_link_libraries(demo-app
    PRIVATE
        Qt5::Widgets
        FGL::Base
)
