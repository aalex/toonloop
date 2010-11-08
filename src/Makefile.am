bin_PROGRAMS = toonloop
EXTRA_DIST = doxyfile.in

html-local: doxygen
doxygen:
	@if [ -z `which doxygen` ]; then echo "No Doxygen!"; else doxygen; fi
clean-local:
	rm -rf html

headers = \
	application.h \
	clip.h \
	concurrentqueue.h \
	config.h \
	configuration.h \
	controller.h \
	gui.h \
	image.h \
	log.h \
	message.h \
	midiinput.h \
	moviesaver.h \
    oscinterface.h \
    oscreceiver.h \
    oscsender.h \
	pipeline.h \
	presets.h \
	saverworker.h \
	statesaving.h \
	subprocess.h \
	timer.h \
	timing.h \
    unused.h \
	v4l2util.h

toonloop_SOURCES = \
	application.cpp \
	clip.cpp \
	configuration.cpp \
	controller.cpp \
	gui.cpp \
	image.cpp \
	main.cpp \
	midiinput.cpp \
	moviesaver.cpp \
	pipeline.cpp \
    presets.cpp \
	saverworker.cpp \
	statesaving.cpp \
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
    -DPIXMAPS_DIR="$(PIXMAPS_DIR)" \
    -DDATADIR="$(datadir)"
 
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
