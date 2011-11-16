#!/usr/bin/env python
"""
Toonloop-launcher uses lunch master as a library to launch toonloop.
"""
if __name__ == "__main__": # just a reminder
    from twisted.internet import gtk2reactor
    gtk2reactor.install() # has to be done before importing reactor
from twisted.internet import reactor
from twisted.internet import task
from lunch import master
# from lunch import gui
from lunch import logger

log = None

class ProcessLauncher(object):
    """
    Process launching with Lunch + a window.

    Might raise a RuntimeError
    """
    def __init__(self, app=None)
        global log
        self.app = app
        self.has_ever_started_it = False
        unique_master_id = "toonloop-launcher"
        log_dir = master.DEFAULT_LOG_DIR
        log_level = 'warning'
        if self.app.config.verbose:
            log_level = 'info'
        if self.app.config.debug:
            log_level = 'debug'
        master.start_logging(log_level=log_level)
        log = logger.start(name="launching")
        pid_file = master.write_master_pid_file(identifier=unique_master_id, directory=log_dir)
        # might raise a RuntimeError:
        self.lunch_master = master.Master(log_dir=log_dir, pid_file=pid_file, verbose=True)
        # self.lunch_gui = gui.start_gui(self.lunch_master)

    def start(self):
        from lunch import commands
        _command = self.app.gui._prepare_toonloop_command()
        if self.lunch_master.has_command("toonloop"):
            self.lunch_master.remove_command("toonloop")
        self.lunch_master.add_command(commands.Command(_command, identifier="toonloop", respawn=False))
        self.lunch_master.restart_all()

    def _onStopped(self):
        pass
