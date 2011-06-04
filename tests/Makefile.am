TESTS = check_looper check_classes check_threads check_bool check_symlinks check_containers check_statesaving check_signals check_imageimporter check_properties
# check_midi check_moviesaver 

noinst_PROGRAMS = $(TESTS)

check_looper_SOURCES = check_looper.cpp
check_looper_CXXFLAGS = $(BOOST_CPPFLAGS)
check_looper_LDADD = $(BOOST_LIBS)

check_classes_SOURCES = check_classes.cpp
check_classes_CXXFLAGS = $(BOOST_CPPFLAGS)
check_classes_LDADD = $(BOOST_LIBS)

check_threads_SOURCES = check_threads.cpp 
check_threads_CXXFLAGS = $(BOOST_CPPFLAGS) 
check_threads_LDADD = $(BOOST_LIBS) $(BOOST_THREAD_LIB)

check_bool_SOURCES = check_bool.cpp 

# check_datetime_SOURCES = check_datetime.cpp ../src/timing.cpp ../src/timer.cpp 
# #../src/timing.h ../src/timer.h
# check_datetime_CXXFLAGS = $(BOOST_CPPFLAGS) $(GTK_CFLAGS) -I../src/ 
# #-I../../src
# check_datetime_LDADD = $(BOOST_LIBS) $(GTK_LIBS)

# check_tween_SOURCES = check_tween.cpp ../src/tween.cpp 
# #../src/tween.h
# check_tween_CXXFLAGS = $(BOOST_CPPFLAGS) -I$(top_srcdir)/src/ 
# #-I../../tests
# check_tween_LDADD = $(BOOST_LIBS) 

check_symlinks_SOURCES = check_symlinks.cpp 
check_symlinks_CXXFLAGS = $(BOOST_CPPFLAGS) 
check_symlinks_LDADD = $(BOOST_LIBS) $(BOOST_THREAD_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB)

# check_moviesaver_SOURCES = check_moviesaver.cpp ../src/clip.cpp  ../src/image.cpp ../src/timing.cpp ../src/moviesaver.cpp ../src/saverworker.cpp ../src/subprocess.cpp 
# #../src/clip.h  ../src/image.h ../src/timing.h ../src/moviesaver.h ../src/saverworker.h ../src/subprocess.h
# check_moviesaver_CXXFLAGS = $(BOOST_CPPFLAGS)  $(GTK_CFLAGS) -I$(top_srcdir)/src/ 
# #-I../../src
# check_moviesaver_LDADD = $(BOOST_LIBS) $(BOOST_THREAD_LIB) $(BOOST_FILESYSTEM_LIB) $(GTK_LIBS)

check_containers_SOURCES = check_containers.cpp 
check_containers_CXXFLAGS = $(BOOST_CPPFLAGS) 
check_containers_LDADD = $(BOOST_LIBS) 

check_statesaving_SOURCES = check_statesaving.cpp 
check_statesaving_CXXFLAGS = $(BOOST_CPPFLAGS) 
check_statesaving_LDADD = $(BOOST_LIBS) 

check_signals_SOURCES = check_signals.cpp 
check_signals_CXXFLAGS = $(BOOST_CPPFLAGS) 
check_signals_LDADD = $(BOOST_LIBS) 

# check_midi_SOURCES = check_midi.cpp ../src/midi.cpp 
# #../src/midi.h
# check_midi_CXXFLAGS = $(STK_CXXFLAGS) $(CXXFLAGS) $(BOOST_CPPFLAGS) -I$(top_srcdir)/src/ 
# #-I../../src
# check_midi_LDADD = $(LDFLAGS) $(STK_LIBS) $(BOOST_LIBS) 

check_imageimporter_SOURCES = check_imageimporter.cpp $(top_srcdir)/src/image_importer.cpp
check_imageimporter_CXXFLAGS = $(BOOST_CPPFLAGS) $(CLUTTERGTK_CFLAGS) -Wno-fatal-error -Wno-deprecated -I$(top_srcdir)/src
check_imageimporter_LDADD = $(BOOST_LIBS) $(CLUTTERGTK_LIBS) $(BOOST_THREAD_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB)

check_properties_SOURCES = $(top_srcdir)/src/properties.h $(top_srcdir)/src/property.h check_properties.cpp
check_properties_CXXFLAGS = -I$(top_srcdir)/src
