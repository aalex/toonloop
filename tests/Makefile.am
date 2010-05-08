TESTS = check_looper check_classes check_commands check_threads check_bool check_datetime
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
check_commands_CXXFLAGS = $(BOOST_CPPFLAGS)
check_commands_LDADD = $(BOOST_LIBS)

check_threads_SOURCES = check_threads.cpp 
check_threads_CXXFLAGS = $(BOOST_CPPFLAGS) 
check_threads_LDADD = $(BOOST_LIBS) $(BOOST_THREAD_LIB)

check_bool_SOURCES = check_bool.cpp 

check_datetime_SOURCES = check_datetime.cpp 
check_datetime_CXXFLAGS = $(BOOST_CPPFLAGS) 
check_datetime_LDADD = $(BOOST_LIBS) 
