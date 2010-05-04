bin_PROGRAMS = toonloop 
#prototoonmidi 

toonloop_SOURCES = main.cpp draw.cpp gui.cpp pipeline.cpp gstgtk.cpp application.cpp clip.cpp videoconfig.cpp image.cpp texture.cpp shader.cpp
toonloop_CXXFLAGS = $(LIBM_CXXFLAGS) $(LIBTHREAD_CXXFLAGS)  $(LIBLO_CXXFLAGS) $(GST_CFLAGS) $(GLEW_CFLAGS) $(BOOST_CPPFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS) 
toonloop_LDADD = $(LIBM_LIBS) $(LIBTHREAD_LIBS) $(LIBLO_LIBS) $(GST_LIBS) $(GLEW_LIBS) $(BOOST_LIBS)  $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_FILESYSTEM_LIB) $(GL_LIBS) 

#prototoonmidi_SOURCES = toonmidi.cpp
#prototoonmidi_CXXFLAGS = $(LIBM_CXXFLAGS) $(LIBTHREAD_CXXFLAGS) $(PORTMIDI_CXXFLAGS) $(ASOUND_CXXFLAGS)
#prototoonmidi_LDADD = $(LIBM_LIBS) $(LIBTHREAD_LIBS) $(PORTMIDI_LIBS) $(ASOUND_LIBS)

