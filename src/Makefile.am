bin_PROGRAMS = toonloop
noinst_PROGRAMS = prototoonmidi 

headers = \
	application.h \
	clip.h \
	config.h \
	configuration.h \
	draw.h \
	gstgtk.h \
	gui.h \
	image.h \
	log.h \
	moviesaver.h \
    oscinterface.h \
    oscreceiver.h \
    oscsender.h \
	pipeline.h \
	subprocess.h \
	texture.h \
	timing.h \
	tween.h \
	v4l2util.h

toonloop_SOURCES = \
	application.cpp \
	clip.cpp \
	configuration.cpp \
	draw.cpp \
	gstgtk.cpp \
	gui.cpp \
	image.cpp \
	main.cpp \
	moviesaver.cpp \
	pipeline.cpp \
	subprocess.cpp \
	texture.cpp \
	timing.cpp \
	tween.cpp \
	v4l2util.cpp \
    oscinterface.cpp \
    oscreceiver.cpp \
    oscsender.cpp \
	$(headers)

toonloop_CXXFLAGS = -Wall \
	$(BOOST_CPPFLAGS) \
	$(CLUTTERGST_CFLAGS) \
	$(CLUTTERGTK_CFLAGS) \
	$(CLUTTER_CFLAGS) \
	$(GLU_CFLAGS) \
	$(GL_CFLAGS) \
	$(GST_CFLAGS) \
	$(GTK_CFLAGS) \
	$(LIBLO_CFLAGS) \
	$(LIBM_CXXFLAGS) \
	$(LIBTHREAD_CXXFLAGS)
 
toonloop_LDADD = \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_LDFLAGS) \
	$(BOOST_LIBS)  \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_THREAD_LIB) \
	$(CLUTTERGST_LIBS) \
	$(CLUTTERGTK_LIBS) \
	$(CLUTTER_LIBS) \
	$(GL_LIBS) \
	$(GST_LIBS) \
	$(GTK_LIBS) \
	$(LIBLO_LIBS) \
	$(LIBM_LIBS) \
	$(LIBTHREAD_LIBS) 

prototoonmidi_SOURCES = toonmidi.cpp pmutil.h
prototoonmidi_CXXFLAGS = $(LIBM_CXXFLAGS) $(LIBTHREAD_CXXFLAGS) $(PORTMIDI_CXXFLAGS) $(ASOUND_CXXFLAGS)
prototoonmidi_LDADD = $(LIBM_LIBS) $(LIBTHREAD_LIBS) $(PORTMIDI_LIBS) $(ASOUND_LIBS)

