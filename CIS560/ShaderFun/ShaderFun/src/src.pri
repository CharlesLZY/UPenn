INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/mainwindow.cpp \
    $$PWD/mygl.cpp \
    $$PWD/utils.cpp \
    $$PWD/la.cpp \
    $$PWD/drawable.cpp \
    $$PWD/scene/quad.cpp \
    $$PWD/shaderprograms/shaderprogram.cpp \
    $$PWD/shaderprograms/postprocessshader.cpp \
    $$PWD/shaderprograms/surfaceshader.cpp \
    $$PWD/texture.cpp \
    $$PWD/scene/camera.cpp \
    $$PWD/tinyobj/tiny_obj_loader.cc \
    $$PWD/scene/mesh.cpp \
    $$PWD/myglslots.cpp \
    $$PWD/cameracontrols.cpp \
    $$PWD/shadercontrols.cpp \
    $$PWD/openglcontext.cpp

HEADERS += \
    $$PWD/la.h \
    $$PWD/mainwindow.h \
    $$PWD/mygl.h \
    $$PWD/utils.h \
    $$PWD/drawable.h \
    $$PWD/scene/quad.h \
    $$PWD/shaderprograms/shaderprogram.h \
    $$PWD/shaderprograms/postprocessshader.h \
    $$PWD/shaderprograms/surfaceshader.h \
    $$PWD/texture.h \
    $$PWD/scene/camera.h \
    $$PWD/tinyobj/tiny_obj_loader.h \
    $$PWD/scene/mesh.h \
    $$PWD/shadercontrols.h \
    $$PWD/openglcontext.h

FORMS +=
