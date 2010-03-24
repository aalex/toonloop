#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Looper Process management.
"""
from toon.launcher import sig
from toon.launcher import process

class Preview(object):
    """
    Local preview - not live
    """
    def __init__(self, app):
        self.app = app
        self.process_manager = None 
        self.state = process.STATE_STOPPED
        self.state_changed_signal = sig.Signal()
            
    def is_busy(self):
        """
        Retuns True if in progress.
        """
        return self.state != process.STATE_STOPPED
        
    def _create_command(self):
        """
        Looks in the settings, and returns a bash command to run the looper.
        @rtype: str
        """
        executable = procutils.which("toonloop")
        args = ["--verbose"]
        command = "%s %s" % (executable, " ".join(args))
        print "$", command
        return command
        
    def start(self):
        print("Starting the looper")
        if self.state != process.STATE_STOPPED:
            raise RuntimeError("Cannot start the looper since it is %s." % (self.state)) # the programmer has done something wrong if we're here.
        else:
            command = self._create_command()
        self.process_manager = process.ProcessManager(command=command, identifier="preview")
        self.process_manager.stdout_line_signal.connect(self.on_stdout_line)
        self.process_manager.stderr_line_signal.connect(self.on_stderr_line)
        self.process_manager.state_changed_signal.connect(self.on_process_state_changed)
        self._set_state(process.STATE_STARTING)
        self.process_manager.start()

    def on_stdout_line(self, process_manager, line):
        print line

    def on_stderr_line(self, process_manager, line):
        print line        
        
    def on_process_state_changed(self, process_manager, process_state):
        """
        Slot for the ProcessManager.state_changed_signal
        """
        print "Preview:", process_manager, process_state
        if process_state == process.STATE_RUNNING:
            # As soon as it is running, set our state to running
            if self.state == process.STATE_STARTING:
                self._set_state(process.STATE_RUNNING)
        elif process_state == process.STATE_STOPPING:
            pass
        elif process_state == process.STATE_STARTING:
            pass
        elif process_state == process.STATE_STOPPED:
            self._set_state(process.STATE_STOPPED)
    
    def _set_state(self, new_state):
        """
        Handles state changes.
        """
        if self.state != new_state:
            self.state_changed_signal(self, new_state)
            self.state = new_state
        else:
            raise RuntimeError("Setting state to %s, which is already the current state." % (self.state))
            
    def stop(self):
        """
        Stops the preview process.
        """
        print("Stopping the looper")
        if self.state in [process.STATE_RUNNING, process.STATE_STARTING]:
            self._set_state(process.STATE_STOPPING)
            if self.process_manager is not None:
                if self.process_manager.state != process.STATE_STOPPED and self.process_manager.state != process.STATE_STOPPING:
                    print('stopping the looper process')
                    self.process_manager.stop()
        else:
            print("Warning: looper state is %s" % (self.state))
