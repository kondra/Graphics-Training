CONFIG   += console

SOURCES += \
    main.cpp \
    main_window.cpp \
    liblinear-1.8/linear.cpp \
    liblinear-1.8/tron.cpp \
    liblinear-1.8/blas/daxpy.c \
    liblinear-1.8/blas/ddot.c \
    liblinear-1.8/blas/dnrm2.c \
    liblinear-1.8/blas/dscal.c \
    logic.cpp

HEADERS += \
    main_window.h \
    liblinear-1.8/linear.h \
    liblinear-1.8/tron.h \
    liblinear-1.8/blas/blas.h \
    liblinear-1.8/blas/blasp.h \
    logic.h

