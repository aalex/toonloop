#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Streamer Process management.
"""
import os
import time
import logging
import signal
from twisted.internet import error
from twisted.internet import protocol
from twisted.internet import reactor
from twisted.python import procutils
from twisted.internet import utils
from scenic import sig
from scenic import configure

# constants for the slave process
STATE_STARTING = "STARTING"
STATE_RUNNING = "RUNNING"
STATE_STOPPING = "STOPPING"
STATE_STOPPED = "STOPPED"

class ProcessError(Exception):
    pass

def run_once(executable, *args):
    """
    Runs a command, without looking at its output or return value.
    Returns a Deferred or None.
    """
    def _cb(result):
        #print(result)
        pass
    try:
        executable = procutils.which(executable)[0]
    except IndexError:
        print("Could not find executable %s" % (executable))
        return None
    else:
        print("Calling %s %s" % (executable, list(args)))
        d = utils.getProcessValue(executable, args, configure.environ_without_custom(), '.', reactor)
        d.addCallback(_cb)
        return d

class ProcessIO(protocol.ProcessProtocol):
    """
    process IO
     
    Its stdout and stderr streams are logged to a file.    
    """
    def __init__(self, manager):
        """
        @param slave: Manager instance.
        """
        self.manager = manager
        self.out_leftover = ""
        self.err_leftover = ""

    def connectionMade(self):
        self.manager._on_connection_made()

    def outReceived(self, data):
        """ 
        Handoff complete lines to manager. Save the leftover line 
        for the next time this is called 
        """
        lines = data.splitlines()
        self.manager.stdout_line_signal(self.manager, self.out_leftover + lines[0])
        for line in lines[1:-1]:
            if line != "":
                self.manager.stdout_line_signal(self.manager, line)
        self.out_leftover = lines[-1]
    
    def errReceived(self, data):
        """ 
        Handoff complete lines to manager. Save the leftover line 
        for the next time this is called 
        """
        lines = data.splitlines()
        self.manager.stderr_line_signal(self.manager, self.err_leftover + lines[0])
        for line in lines[1:-1]:
            if line != "":
                self.manager.stderr_line_signal(self.manager, line)
        self.err_leftover = lines[-1]

    def processEnded(self, reason):
        exit_code = reason.value.exitCode
        if exit_code is None:
            exit_code = reason.value.signal
        self.manager._on_process_ended(exit_code)
    
    def processExited(self, reason):
        self.manager.log("process has exited " + str(reason.value))
    
class ProcessManager(object):
    """
    Manages a streamer process. 
    """
    def __init__(self, command=None, identifier=None, env=None):
        """
        @param command: Shell string. The first item is the name of the name of the executable.
        @param identifier: Any string. 
        """
        #Used as a file name, so avoid spaces and exotic characters.
        self._process_transport = None
        self._child_process = None
        self._time_child_started = None
        self._child_running_time = None
        self.state = STATE_STOPPED
        self.command = command # string (bash)
        self.time_before_sigkill = 5.0 # seconds
        self.identifier = identifier # title
        self.env = {} # environment variables for the child process
        if env is not None:
            self.env.update(env)
        self.pid = None
        if self.identifier is None:
            self.identifier = "default"
        self.log_level = logging.DEBUG
        self._delayed_kill = None # DelayedCall instance
        
        self.state_changed_signal = sig.Signal()
        self.stdout_line_signal = sig.Signal()
        self.stderr_line_signal = sig.Signal()
    
    def _before_shutdown(self):
        """
        Called before twisted's reactor shutdown.
        to make sure that the process is dead before quitting.
        """
        if self.state in [STATE_STARTING, STATE_RUNNING, STATE_STOPPING]:
            msg = "Child still %s. Stopping it before shutdown." % (self.state)
            self.log(msg)
            self.stop()
    
    def is_alive(self):
        """
        Checks if the child is alive.
        """
        #TODO Use this
        if self.state == STATE_RUNNING:
            proc = self._process_transport
            try:
                proc.signalProcess(0)
            except (OSError, error.ProcessExitedAlready):
                msg = "Lost process %s. Error sending it an empty signal." % (self.identifier)
                print(msg)
                return False
            else:
                return True
        else:
            return False
    
    def start(self):
        """
        Starts the child process
        """
        if self.state in [STATE_RUNNING, STATE_STARTING]:
            msg = "Child is already %s. Cannot start it." % (self.state)
            raise ProcessError(msg)
        elif self.state == STATE_STOPPING:
            msg = "Child is %s. Please try again to start it when it will be stopped." % (self.state)
            raise ProcessError(msg)
        if self.command is None or self.command.strip() == "":
            msg = "You must provide a command to be run."
            raise ProcessError(msg)
        
        self.log("Will run command %s %s" % (self.identifier, str(self.command)))
        self._child_process = ProcessIO(self)
        environ = {}
        environ.update(configure.environ_without_custom())
        for key, val in self.env.iteritems():
            environ[key] = val
        self.set_child_state(STATE_STARTING)
        shell = "/bin/sh"
        if os.path.exists("/bin/bash"):
            shell = "/bin/bash"
        self._time_child_started = time.time()
        self._process_transport = reactor.spawnProcess(self._child_process, shell, [shell, "-c", "exec %s" % (self.command)], environ)
        self.pid = self._process_transport.pid
        self.log("Spawned child %s with pid %s." % (self.identifier, self.pid))
    
    def _on_connection_made(self):
        if not STATE_STARTING:
            self.log("Connection made even if we were not starting the child process.", logging.ERROR)
        self.set_child_state(STATE_RUNNING)
    
    def stop(self):
        """
        Stops the child process
        """
        def _later_check(pid):
            if self.pid == pid:
                if self.state == STATE_STOPPING:
                    msg = "Child process %s not dead." % (self.identifier)
                    print msg
                    try:
                        self._process_transport.signalProcess(signal.SIGKILL)
                    except OSError, e:
                        msg = "Error sending signal %s to process %s. %s" % (signal_to_send, self.identifier, e)
                        print msg # raise?
                    except error.ProcessExitedAlready:
                        msg = "Process %s had already exited while trying to send signal %s." % (self.identifier, "SIGKILL")
                        print msg # raise ?
                elif self.state == STATE_STOPPED:
                    msg = "Successfully killed process after least than the %f seconds. State is %s." % (self.time_before_sigkill, self.state)
                    self.log(msg)
            self._delayed_kill = None
        
        # TODO: do callLater calls to check if the process is still running or not.
        #see twisted.internet.process._BaseProcess.reapProcess
        signal_to_send = None
        if self.state in [STATE_RUNNING, STATE_STARTING]:
            self.set_child_state(STATE_STOPPING)
            self.log('Will stop process using SIGTERM.')
            signal_to_send = signal.SIGTERM
        elif self.state == STATE_STOPPING:
            self.log('Trying to kill again the child process using SIGKILL.')
            signal_to_send = signal.SIGKILL
        else: # STOPPED
            msg = "Process is already stopped."
            self.set_child_state(STATE_STOPPED)
            print msg # raise?
        if signal_to_send is not None:
            try:
                self._process_transport.signalProcess(signal_to_send)
            except OSError, e:
                msg = "Error sending signal %s to process %s. %s" % (signal_to_send, self.identifier, e)
                print msg # raise?
            except error.ProcessExitedAlready:
                if signal_to_send == signal.SIGTERM:
                    msg = "Process %s had already exited while trying to send signal %s." % (self.identifier, signal_to_send)
                    print msg # raise ?
            else:
                if signal_to_send == signal.SIGTERM:
                    self._delayed_kill = reactor.callLater(self.time_before_sigkill, _later_check, self.pid)

    def log(self, msg, level=logging.DEBUG):
        """
        Logs to Master.
        (through stdout)
        """
        if level >= self.log_level:
            print "%9s process: %s" % (self.identifier, msg)

    def _on_process_ended(self, exit_code):
        self._child_running_time = time.time() - self._time_child_started
        if self.state == STATE_STOPPING:
            self.log('Child process exited as expected.')
            if self._delayed_kill is not None:
                if self._delayed_kill.active:
                    self._delayed_kill.cancel()
                self._delayed_kill = None
        elif self.state == STATE_STARTING:
            self.log('Child process exited while trying to start it.')
        elif self.state == STATE_RUNNING:
            if exit_code == 0:
                self.log('Child process exited.')
            else:
                self.log('Child process exited with error.')
        self._process_transport.loseConnection() # close file handles
        self.log("Child exitted with %s" % (exit_code), logging.INFO)
        self.set_child_state(STATE_STOPPED)
        
    def set_child_state(self, new_state):
        """
        Handles state changes.
        """
        if self.state != new_state:
            if new_state == STATE_STOPPED:
                self.log("Child lived for %s seconds." % (self._child_running_time))
                #self.io_protocol.send_state(new_state, self._child_running_time)
            elif self.state == STATE_STOPPED and new_state != STATE_STARTING:
                raise RuntimeError("Cannot go from STATE_STOPPED to %s " % (new_state))
            self.state_changed_signal(self, new_state)
            self.log("child state: %s" % (new_state))
        else:
            self.log("State is same as before: %s" % (new_state))
        self.state = new_state

    def __str__(self):
        return "%s %s" % (self.identifier, id(self))
