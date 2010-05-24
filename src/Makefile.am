bin_PROGRAMS = toonloop prototoonmidi 

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
    oscinterface.h \
    oscreceiver.h \
    oscsender.h \
	pipeline.h \
	shader.h \
	subprocess.h \
	texture.h \
	timing.h

toonloop_SOURCES = \
	application.cpp \
	clip.cpp \
	configuration.cpp \
	draw.cpp \
	gstgtk.cpp \
	gui.cpp \
	image.cpp \
	main.cpp \
    oscinterface.cpp \
    oscreceiver.cpp \
    oscsender.cpp \
	pipeline.cpp \
	shader.cpp \
	texture.cpp \
	timing.cpp \
	$(headers)

toonloop_CXXFLAGS = \
	$(BOOST_CPPFLAGS) \
	$(GLEW_CFLAGS) \
	$(GLU_CFLAGS) \
	$(GL_CFLAGS) \
	$(GSTGL_CFLAGS) \
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
	$(GLEW_LIBS) \
	$(GL_LIBS) \
	$(GSTGL_LIBS) \
	$(GST_LIBS) \
	$(GTK_LIBS) \
	$(LIBLO_LIBS) \
	$(LIBM_LIBS) \
	$(LIBTHREAD_LIBS) 

prototoonmidi_SOURCES = toonmidi.cpp
prototoonmidi_CXXFLAGS = $(LIBM_CXXFLAGS) $(LIBTHREAD_CXXFLAGS) $(PORTMIDI_CXXFLAGS) $(ASOUND_CXXFLAGS)
prototoonmidi_LDADD = $(LIBM_LIBS) $(LIBTHREAD_LIBS) $(PORTMIDI_LIBS) $(ASOUND_LIBS)

