/**
 * Compile with:
 * gcc -Wall `pkg-config --libs --cflags glib-2.0` -o spawn spawn.c
 */
#include <glib.h>
//#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>

class Subprocess
{
    public:
        Subprocess(std::vector<std::string> args);
        bool run();
        std::string poll();
        bool isRunning();
        virtual ~Subprocess();
        void setRunning(bool running)
        {
            is_running_ = running;
        }
    private:
        std::vector<std::string> args_;
        bool is_running_;
        int std_output_;
        int pid_; // FIXME doesn't work on win32
        GIOChannel *out_channel_;
        void cb_child_watch(GPid pid, gint status, gpointer data);
};

Subprocess::Subprocess(std::vector<std::string> args) :
    args_(args),
    is_running_(false)
{
}

bool Subprocess::isRunning()
{
    // FIXME
    return is_running_;
}

void Subprocess::cb_child_watch(GPid pid, gint status, gpointer data)
{
    Subprocess *context = static_cast<Subprocess*>(data);
    context->setRunning(false);
    // g_spawn_close_pid(pid); // useless on POSIX
}

bool Subprocess::run(void)
{
    GError *err;
    err = NULL;
    // TODO: use args_
    gchar *args[] = {
        "sh",
        "-c",
        "echo hello",
        NULL
        };
    GSpawnFlags flags = G_SPAWN_SEARCH_PATH;
    //GPid pid;
    if (! g_spawn_async_with_pipes(
        NULL, /* inherit working directory */
        args,
        NULL, /* inherit env vars */
        flags,
        NULL, /* function to call after fork(), before exec() */
        NULL, /* user data */
        &pid_,
        NULL,
        &std_output_,
        NULL,
        &err))
    {
        g_critical("Error: %s\n", err->message);
        g_error_free(err);
        return false;
    }
    else
    {
        g_print("success\n");    
    }

    /* Add watch function to catch termination of the process. This function
     * will clean any remnants of process. */
    //g_child_watch_add(pid_, (GChildWatchFunc) &cb_child_watch, this);

    /* Create channels that will be used to read data from pipes. */
    // FIXME doesn't work on win32
    out_channel_ = g_io_channel_unix_new(std_output_);
    is_running_ = true;
    return true;
}

std::string Subprocess::poll()
{
    std::string ret = "";
    if (isRunning())
    {
        gchar *string;
        gsize size;
        g_io_channel_read_line(out_channel_, &string, &size, NULL, NULL);
        if (size > 0)
        {
            std::cout << "read " << size << " bytes:" << string << std::endl;
            ret = string;
            g_free(string);
        }
    }
    else
        std::cerr << "Subprocess::" << __FUNCTION__ << ": not running" << std::endl;
    return ret;
}

Subprocess::~Subprocess()
{
    // TODO: close std_output socket
    // TODO: kill child if still alive
    //g_spawn_close_pid(pid_);
}

int main(int argc, char *argv[])
{
    std::vector<std::string> args;
    Subprocess proc = Subprocess(args);
    proc.run();
    for (int i = 0; i < 20; ++i)
        proc.poll();
    return 0;
}

