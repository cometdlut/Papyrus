
android {
    manifest.source = android-build
    manifest.target = .
    COPYFOLDERS += manifest
    include(qmake/copyData.pri)
    copyData()

    QT += androidextras
    SOURCES += \
        sialantools/sialanjavalayer.cpp

    HEADERS += \
        sialantools/sialanjavalayer.h
}

SOURCES += \
    sialantools/sialandevices.cpp \
    sialantools/sialanqtlogger.cpp \
    sialantools/sialantools.cpp \
    sialantools/sialandesktoptools.cpp \
    sialantools/sialanlistobject.cpp \
    sialantools/sialanhashobject.cpp

HEADERS += \
    sialantools/sialandevices.h \
    sialantools/sialanqtlogger.h \
    sialantools/sialantools.h \
    sialantools/sialandesktoptools.h \
    sialantools/sialanlistobject.h \
    sialantools/sialanhashobject.h

OTHER_FILES += \
    sialantools/android-build/src/org/sialan/android/SialanActivity.java \
    sialantools/android-build/src/org/sialan/android/SialanApplication.java \
    sialantools/android-build/src/org/sialan/android/SialanJavaLayer.java
