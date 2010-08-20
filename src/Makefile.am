bin_PROGRAMS = toonloop
# noinst_PROGRAMS = 

headers = \
	application.h \
	clip.h \
	config.h \
	configuration.h \
	gui.h \
	image.h \
	log.h \
	moviesaver.h \
    oscinterface.h \
    oscreceiver.h \
    oscsender.h \
	pipeline.h \
	subprocess.h \
	timer.h \
	timing.h \
	v4l2util.h

toonloop_SOURCES = \
	application.cpp \
	clip.cpp \
	configuration.cpp \
	gui.cpp \
	image.cpp \
	main.cpp \
	moviesaver.cpp \
	pipeline.cpp \
	subprocess.cpp \
	timer.cpp \
	timing.cpp \
	v4l2util.cpp \
    oscinterface.cpp \
    oscreceiver.cpp \
    oscsender.cpp \
	$(headers)

toonloop_CXXFLAGS = \
	$(CXXFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(CLUTTERGST_CFLAGS) \
	$(CLUTTERGTK_CFLAGS) \
	$(LIBLO_CFLAGS) \
	$(LIBM_CXXFLAGS) \
	$(LIBTHREAD_CXXFLAGS) \
	$(STK_CXXFLAGS) \
	$(CXXFLAGS) \
	-DPIXMAPS_DIR=\"$(PIXMAPS_DIR)\"
 
toonloop_LDADD = \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_LDFLAGS) \
	$(BOOST_LIBS)  \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_THREAD_LIB) \
	$(CLUTTERGST_LIBS) \
	$(CLUTTERGTK_LIBS) \
	$(LIBLO_LIBS) \
	$(LIBM_LIBS) \
	$(LIBTHREAD_LIBS) \
	$(LDFLAGS) \
	$(STK_LIBS)

