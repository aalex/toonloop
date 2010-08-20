TESTS = check_looper check_classes check_commands check_threads check_bool check_datetime check_tween check_moviesaver check_symlinks check_containers check_statesaving check_signals check_midi

noinst_PROGRAMS = $(TESTS)

check_looper_SOURCES = check_looper.cpp
check_looper_CXXFLAGS = $(BOOST_CPPFLAGS)
check_looper_LDADD = $(BOOST_LIBS)

check_classes_SOURCES = check_classes.cpp
check_classes_CXXFLAGS = $(BOOST_CPPFLAGS)
check_classes_LDADD = $(BOOST_LIBS)

#check_files_SOURCES = check_files.cpp
#check_files_CXXFLAGS = $(BOOST_CPPFLAGS)
#check_files_LDADD = $(BOOST_LIBS) $(BOOST_FILESYSTEM_LIB)

check_commands_SOURCES = check_commands.cpp ../src/subprocess.cpp
check_commands_CXXFLAGS = $(BOOST_CPPFLAGS) -I ../src/
check_commands_LDADD = $(BOOST_LIBS)

check_threads_SOURCES = check_threads.cpp 
check_threads_CXXFLAGS = $(BOOST_CPPFLAGS) 
check_threads_LDADD = $(BOOST_LIBS) $(BOOST_THREAD_LIB)

check_bool_SOURCES = check_bool.cpp 

check_datetime_SOURCES = check_datetime.cpp ../src/timing.cpp ../src/timer.cpp
check_datetime_CXXFLAGS = $(BOOST_CPPFLAGS) $(GTK_CFLAGS) -I ../src/
check_datetime_LDADD = $(BOOST_LIBS) $(GTK_LIBS)

check_tween_SOURCES = check_tween.cpp ../src/tween.cpp
check_tween_CXXFLAGS = $(BOOST_CPPFLAGS) -I ../src/
check_tween_LDADD = $(BOOST_LIBS) 

check_symlinks_SOURCES = check_symlinks.cpp 
check_symlinks_CXXFLAGS = $(BOOST_CPPFLAGS) 
check_symlinks_LDADD = $(BOOST_LIBS) $(BOOST_THREAD_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB)


check_moviesaver_SOURCES = check_moviesaver.cpp ../src/clip.cpp  ../src/image.cpp ../src/timing.cpp ../src/moviesaver.cpp ../src/subprocess.cpp
check_moviesaver_CXXFLAGS = $(BOOST_CPPFLAGS) $(GTK_CFLAGS) -I ../src/
check_moviesaver_LDADD = $(BOOST_LIBS) $(BOOST_THREAD_LIB) $(GTK_LIBS)

check_containers_SOURCES = check_containers.cpp 
check_containers_CXXFLAGS = $(BOOST_CPPFLAGS) 
check_containers_LDADD = $(BOOST_LIBS) 


check_statesaving_SOURCES = check_statesaving.cpp 
check_statesaving_CXXFLAGS = $(BOOST_CPPFLAGS) 
check_statesaving_LDADD = $(BOOST_LIBS) 

check_signals_SOURCES = check_signals.cpp 
check_signals_CXXFLAGS = $(BOOST_CPPFLAGS) 
check_signals_LDADD = $(BOOST_LIBS) $(BOOST_SIGNALS_LIB)

check_midi_SOURCES = check_midi.cpp ../src/midi.cpp
check_midi_CXXFLAGS = $(STK_CXXFLAGS) $(CXXFLAGS)
check_midi_LDADD = $(LDFLAGS) $(STK_LIBS)

