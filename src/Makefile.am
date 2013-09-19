SUBDIRS = shaders
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
	clutter-tools.h \
	command.h \
	concurrentqueue.h \
	config.h \
	configuration.h \
	controller.h \
	gui.h \
	image.h \
	image_importer.h \
	legacyclutter.h \
	log.h \
	midibinder.h \
	midiinput.h \
	moviesaver.h \
	pipeline.h \
	properties.h \
	property.h \
	raw1394util.h \
	saverworker.h \
	statesaving.h \
	subprocess.h \
	timer.h \
	timing.h \
	brcosaeffect.h \
	effect.h \
	glslang.h \
	infowindow.h \
	oscinterface.h \
	oscreceiver.h \
	oscsender.h \
	playheaditerator.h \
	unused.h \
	v4l2util.h

toonloop_SOURCES = \
	application.cpp \
	clip.cpp \
	clutter-tools.cpp \
	command.cpp \
	configuration.cpp \
	controller.cpp \
	gui.cpp \
	image.cpp \
	image_importer.cpp \
	main.cpp \
	midiinput.cpp \
	moviesaver.cpp \
	pipeline.cpp \
	raw1394util.cpp \
	saverworker.cpp \
	subprocess.cpp \
	timer.cpp \
	timing.cpp \
	v4l2util.cpp \
	brcosaeffect.cpp \
	effect.cpp \
	glslang.cpp \
	infowindow.cpp \
	midibinder.cpp \
	oscinterface.cpp \
	oscreceiver.cpp \
	oscsender.cpp \
	playheaditerator.cpp \
	$(headers)

toonloop_CXXFLAGS = \
	$(CXXFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(CLUTTERGST_CFLAGS) \
	$(LIBLO_CFLAGS) \
	$(LIBM_CXXFLAGS) \
	$(LIBTHREAD_CXXFLAGS) \
	$(RAW1394_CFLAGS) \
	$(AVC1394_CFLAGS) \
	$(RTMIDI_CFLAGS) \
	$(LIBXML_CFLAGS)
 
toonloop_LDADD = \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_DATE_TIME_LIBS) \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(BOOST_SIGNALS_LIBS) \
	$(BOOST_SYSTEM_LIBS) \
	$(BOOST_THREAD_LIBS) \
	$(CLUTTERGST_LIBS) \
	$(LIBLO_LIBS) \
	$(LIBM_LIBS) \
	$(LIBTHREAD_LIBS) \
	$(LDFLAGS) \
	$(RAW1394_LIBS) \
	$(AVC1394_LIBS) \
	$(RTMIDI_LIBS) \
	$(LIBXML_LIBS)

