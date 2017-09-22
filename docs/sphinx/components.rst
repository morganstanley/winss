.. _components:

==========
Components
==========

.. important::

   Some components like :ref:`winss-svscan` require the **PATH** environment
   to be set correctly. To run these commands please append the install
   directory to the **PATH**.

.. _winss-supervise:

winss-supervise.exe
-------------------

:ref:`winss-supervise` monitors a :term:`service`, making sure it stays alive,
sending notifications to registered processes when it dies, and providing an
interface to control its state.

.. code-block:: bat

   Usage: winss-supervise.exe [options] servicedir

   Options:
     --help       Print usage and exit.
     --version    Print the current version of winss and exit.
     -v[<level>], --verbose[=<level>]
                       Sets the verbose level

- :ref:`winss-supervise` changes directory to ``servicedir``
  :term:`service directory`.
- It exits **100** if another :ref:`winss-supervise` process is already 
  monitoring this :term:`service`.
- If the default state is *up* and not :ref:`down` then :ref:`winss-supervise`
  starts the :ref:`run` process.
- If the :ref:`env` dir exists then a new environment block will be constructed
  and the :ref:`run` process will be started with the new environment block.
- If the :ref:`run` process fails to start then it will wait 10 seconds
  before trying to start again. It does not execute :ref:`finish` on failure
  to execute :ref:`run`.
- When :ref:`run` dies, :ref:`winss-supervise` will start the :ref:`finish`
  process if it exists, with the exit code of :ref:`run`. The following
  environment variables will be set:

  .. envvar:: SUPERVISE_RUN_EXIT_CODE
  
    The exit code of the :ref:`run` process will be set for the :ref:`finish`
    process.

- By default, :ref:`finish` must exit in less than *5-seconds* and will be
  terminated if still running. This timeout can be customized using the
  :ref:`timeout-finish` file.
- When :ref:`finish` dies (or is killed), :ref:`winss-supervise` will wait at
  least *1-second* before starting :ref:`run` again to avoid busy-looping if
  :ref:`run` exits too quickly.
- If :ref:`finish` exits with 125, then :ref:`winss-supervise` will not restart
  the :ref:`run` process. This can be used to signify permanent failure to
  start the service or you want to control the service coming up manually.

.. note::

   The :ref:`run` process will be sent a **CTRL-BREAK** signal_ when it is asked
   to exit. By default the **CTRL-BREAK** will exit the program but it can be
   handled and used to exit the program cleanly.

.. seealso::

   :ref:`winss-svc`
      Can be used to send commands to the :ref:`winss-supervise` process;
      mostly to change the :term:`service` state.

   :ref:`winss-svok`
      Can be used to check whether a :ref:`winss-supervise` is successfully
      running.

   :ref:`winss-svstat`
      Can be used to check the status of a :term:`service`.

.. _winss-svscan:

winss-svscan.exe
----------------

:ref:`winss-svscan` starts and monitors a collection of :ref:`winss-supervise`
processes in a :term:`scan directory`, each of these processes monitoring a
single :term:`service`. It is designed to be either the root or a branch of a
*supervision tree*.

.. code-block:: bat

   Usage: winss-svscan.exe [options] [scandir]                            
                                                                       
   Options:                                                               
     --help       Print usage and exit.                                   
     --version    Print the current version of winss and exit.            
     -v[<level>], --verbose[=<level>]                                     
                       Sets the verbose level.                            
     -t<rescan>,  --timeout=<rescan>                                      
                       Sets the rescan timeout.                           

- If given a ``scandir`` is specified then that is used. Otherwise then the
  current directory is used.
- It exits **100** if another :ref:`winss-svscan` process is already 
  monitoring this :term:`scan directory`.
- If the **./.winss-svscan** control directory does not exist,
  :ref:`winss-svscan` creates it. However, it is recommended to already have a
  .winss-svscan subdirectory in your :term:`scan directory` directory, because
  :ref:`winss-svscan` may try to launch **.winss-svscan/finish** at some point.
- If the :ref:`env` dir exists within **./.winss-svscan** then the current
  environment will be applied to the scan process.
- :ref:`winss-svscan` performs an initial scan of its scan directory.
- :ref:`winss-svscan` then occasionally runs scans based on the timeout
  specified or asked to do so by :ref:`winss-svscanctl`.
- :ref:`winss-svscan` runs until it is told to stop via :ref:`winss-svscanctl`,
  or a signal. Then it starts the **.winss-svscan/finish** program.

Options
^^^^^^^

 -t<rescan>\, --timeout=<rescan> 
    Perform a scan every ``rescan`` milliseconds. If rescan is **0**
    (the default), automatic scans are never performed after the first one and
    :ref:`winss-svscan` will only detect new :term:`services <service>` when
    told to via a :ref:`winss-svscanctl` -a command. It is strongly discouraged
    to set ``rescan`` to a positive value under **500**.

Scan
^^^^

Every ``rescan`` milliseconds, or upon receipt of a :ref:`winss-svscanctl` -a
command, :ref:`winss-svscan` runs a scanner routine.

The scanner scans the current directory for subdirectories (or symbolic links
to directories), which must be :term:`service directories <service directory>`.
It skips names starting with dots.

For every new subdirectory dir it finds, the scanner spawns a
:ref:`winss-supervise` process on it. If **dir/log** exists, it spawns a
:ref:`winss-supervise` process on both **dir** and **dir/log**, and creates a
pipe from the service's stdout to the logger's stdin. This is starting the
:term:`service`, with or without a corresponding logger. Every :term:`service`
the scanner finds is flagged as "active".

The scanner remembers the :term:`services <service>` it found. If a
:term:`service` has been started in an earlier scan, but the current scan can't
find the corresponding directory, the :term:`service` is then flagged as
inactive. No command is sent to stop inactive :ref:`winss-supervise` processes
(unless the administrator uses :ref:`winss-svscanctl` -n), but inactive
:ref:`winss-supervise` processes will not be restarted if they die.

.. note::

   :ref:`winss-supervise` is used by :ref:`winss-svscan` and must be in the
   **PATH**.

.. seealso::

   :ref:`winss-svscanctl`
      Can be used to send commands to the :ref:`winss-svscan` process;
      mostly to signal a rescan.

.. _winss-log:

winss-log.exe
-------------

:ref:`winss-log` is a reliable logging program with automated log rotation.

.. code-block:: bat

   Usage: winss-log.exe [options] script

   Options:
     --help       Print usage and exit.
     --version    Print the current version of winss and exit.
     -v[<level>], --verbose[=<level>]
                       Sets the verbose level.

:ref:`winss-log` reads and compiles logging script to an internal form. Then it
reads its standard input, line by line, and performs actions on it, following
the script it is given. It does its best to ensure there is never any log loss.
It exits cleanly when stdin closes.

.. note::

   The current logging script is limited to a single set of settings which can
   rotate files which exceed size ``s``, keep ``n`` backups and output to a
   single *logdir*.

Logdirs
^^^^^^^

A *logdir (logging directory)* is a place where logs are stored. Currently
:ref:`winss-log` can only be configured to output to a single directory.

A *logdir* may contain the following files:

- **current**: the file where the current log stream is appended to.
- **@timestamp.u**: old log files which have been rotated.

Rotation
""""""""

When the **current** file gets too big then a *rotation* occurs. The *archived*
log file will be in the form *@timestamp.u* where *timestamp* is the number of
seconds since the epoch. If there are too many archived log files in the
*logdir*, the older ones are then removed. The logging stream will continue to
log to a brand new **current** file.

Script
^^^^^^

When starting up, :ref:`winss-log` reads its arguments one by one; this argument sequence, or *directive sequence*, forms a *logging script* which tells
:ref:`winss-log` what to log, where, and how.

Every directive can be a *control directive* or an *action directive*. A valid
logging script always contains at least one *action directive*; every *action
directive* can be preceded by zero or more *control directives*.
:ref:`winss-log` will exit 100 if the script is invalid.

Control
"""""""

These directives tune :ref:`winss-log`'s behavior for the next actions.

- **n** *number*: next logdirs will contain up to *number* archived log files.
  If there are more, the oldest archived log files will be suppressed, only the
  latest *number* will be kept. By default, *number* is 10.
- **s** *filesize*: next rotations will occur when current log files approach
  *filesize* bytes. By default, *filesize* is 99999; it cannot be set lower than
  4096 or higher than 16777215.
- **T**: the selected line will be prepended with a
  `ISO 8601 timestamp <iso_timestamp>`_.

Action
""""""

These directives determine what :ref:`winss-log` actually does with the logs.

- **dir** (must start with '.' or '[A-Z]:'): logdir. :ref:`winss-log` will
  log the line into the log *dir*. :ref:`winss-log` must have the right to write
  to the log *dir*.
    
   The drive letter needs to be different from a control directive otherwise
   it will not be interpreted as a log *dir*. Unfortunately UNC paths are not
   supported at this time but this will solve this issue.

Examples
""""""""

:ref:`winss-log` n20 s1000000 .

.. _winss-svc:

winss-svc.exe
-------------

:ref:`winss-svc` sends commands to a running :ref:`winss-supervise` process.
In other words, it's used to control a supervised process.

.. code-block:: bat

   Usage: winss-svc.exe [options] servicedir

   Options:
     --help       Print usage and exit.
     --version    Print the current version of winss and exit.
     -v[<level>], --verbose[=<level>]
                       Sets the verbose level.
     -k,          --kill
                       Terminate the process.
     -t,          --term
                       Send a CTRL+BREAK to the process
     -o,          --once
                       Equivalent to '-uO'.
     -d,          --down
                       Stop the supervised process.
     -u,          --up
                       Starts the supervised process.
     -x,          --exit
                       Stop the process and supervisor.
     -O,          --onceatmost
                       Only run supervised process once.
     -T<ms>,      --timeout=<ms>
                       Wait timeout in milliseconds if -w is specified.
     -w<dDur>,    --wait=<dDur>
                       Wait on (d)own/finishe(D)/(u)p/(r)estart.

:ref:`winss-svc` sends the given series of commands in the order given to the
:ref:`winss-supervise` process monitoring the :term:`service directory`, then
exits 0. It exists 111 if it cannot send a command, or 100 if no
:ref:`winss-supervise` process is running on :term:`service directory`

Options
^^^^^^^

 -k\, --kill
    Instruct the supervisor to kill the supervised process.
 -t\, --term
    Instruct the supervisor to send a :kbd:`Control-Break` to the supervised
    process.
 -o\, --once
    Equivalent to "-uO".
 -d\, --down
    If the supervised process is up, send it a :kbd:`Control-Break`. Do not
    restart it.
 -u\, --up
    If the supervised process is down, start it. Automatically restart it when
    it dies.
 -x\, --exit
    When the service is asked to be down and the supervised process dies, 
    :ref:`winss-supervise` will exit too. This command should normally never be
    used on a working system.
 -O\, --onceatmost
    Do not restart the supervised process when it dies. If it is down when the
    command is received, do not even start it.
 -t<ms>\, --timeout=<ms> 
    If the -wstate option has been given, -T specifies a timeout
    (in milliseconds) after which :ref:`winss-svc` will exit 1 with an error
    message if the service still hasn't reached the desired state. By default,
    the timeout is 0, which means that :ref:`winss-svc` will block indefinitely.
 -wd\, --wait=d
    :ref:`winss-svc` will not exit until the :term:`service` is down, i.e. until
    the :ref:`run` process has died.
 -wD\, --wait=D
    :ref:`winss-svc` will not exit until the :term:`service` is *down* and
    *ready* to be brought *up*, i.e. a possible :ref:`finish` script has
    exited.
 -wu\, --wait=u
    :ref:`winss-svc` will not exit until the :term:`service` is *up*, i.e. there
    is a process running the :ref:`run` executable.
 -wr\, --wait=r
    :ref:`winss-svc` will not exit until the :term:`service` has been started or
    restarted.

.. seealso::

   :ref:`winss-svwait`
      Can be used to wait on the :ref:`winss-supervise` process without sending
      any commands.


.. _winss-svok:

winss-svok.exe
--------------

:ref:`winss-svok` checks whether a :term:`service directory` is currently
supervised.

.. code-block:: bat

   Usage: winss-svok.exe [options] servicedir

   Options:
     --help       Print usage and exit.
     --version    Print the current version of winss and exit.
     -v[<level>], --verbose[=<level>]
                       Sets the verbose level.

:ref:`winss-svok` exits 0 if there is a :ref:`winss-supervise` process
monitoring the *servicedir* :term:`service directory`, or 1 if there is not.

.. _winss-svstat:

winss-svstat.exe
----------------

:ref:`winss-svstat` prints a short, human-readable summary of the state of a
process monitored by :ref:`winss-supervise`.

.. code-block:: bat

   Usage: winss-svstat.exe [options] servicedir

   Options:
     --help       Print usage and exit.
     --version    Print the current version of winss and exit.
     -v[<level>], --verbose[=<level>]
                       Sets the verbose level.

:ref:`winss-svstat` gives information about the process being monitored at
the *servicedir* :term:`service directory`, then exits 0. The information
includes the following:

- whether the process is up or down, and if it's up, the number of seconds that
  it has been up.
- the process' pid, if it is up, or its last exit code or terminating signal,
  if it is down.
- what its default state is, if it is different from its current state.
- the number of seconds since it last changed states.
- whether the A :term:`service` is ready and if it is, the number of seconds
  that it has been. A A :term:`service` reported as down and ready simply means
  that it is ready to be brought up. A :term:`service` is down and not ready
  when it is in the cleanup phase, i.e. the :ref:`finish` script is still being
  executed.


Exit Codes
^^^^^^^^^^

- 0: success
- 1: :ref:`winss-supervise` not running on *servicedir*
  :term:`service directory`
- 100: wrong usage
- 111: system call failed

.. _winss-svwait:

winss-svwait.exe
----------------

:ref:`winss-svwait` blocks until a collection of supervised services goes up,
or down.

:ref:`winss-svwait` only waits for notifications; it never polls.

.. code-block:: bat

   Usage: winss-svwait.exe [options] servicedir

   Options:
     --help       Print usage and exit.
     --version    Print the current version of winss and exit.
     -v[<level>], --verbose[=<level>]
                       Sets the verbose level.
     -u,          --up
                       Wait until the services are up.
     -d,          --down
                       Wait until the services are down.
     -D,          --finished
                       Wait until the services are really down.
     -o,          --or
                       Wait until one of the services comes up or down.
     -a,          --and
                       Wait until all of the services comes up or down.
     -t<ms>,      --timeout=<ms>
                       Wait timeout in milliseconds.

:ref:`winss-svwait` monitors one or more
:term:`service directories <service directory>` given as its arguments, waiting
for a state (ready, up or down) to happen. It exits 0 when the wanted
condition becomes true.

Options
^^^^^^^

 -u\, --up
    :ref:`winss-svwait` will wait until the :term:`services <service>` are up,
    as reported by :ref:`winss-supervise`. This is the default; it is not
    reliable, but it does not depend on specific support in the service
    programs.
 -d\, --down
    :ref:`winss-svwait` will wait until the :term:`services <service>` are down.
 -D\, --finished
    :ref:`winss-svwait` will wait until the :term:`services <service>` are down
    and the cleanup scripts in :ref:`finish` for every *servicedir* have
    finished executing (or have timed out and been killed).
 -o\, --or
    :ref:`winss-svwait` will wait until *one* of the given
    :term:`services <service>` comes up or down.
 -a\, --and
    :ref:`winss-svwait` will wait until *all* of the given
    :term:`services <service>` comes up or down. This is the default.
 -t<ms>\, --timeout=<ms> 
    If the requested events have not happened after *timeout* milliseconds,
    :ref:`winss-svwait` will print a message to stderr and exit 1.
    By default, *timeout* is 0, which means no time limit.

.. note::

   - :ref:`winss-svwait` should be given one or more
     :term:`service directories <service directory>` as arguments, not a
     :term:`scan directory`. If you need to wait for a whole
     :term:`scan directory`, give all its contents as arguments to
     :ref:`winss-svwait`.
   - :ref:`winss-svwait` will only work on
     :term:`service directories <service directory>` that are already active,
     i.e. have a :ref:`winss-supervise` process running on them. It will not
     work on a :term:`service directory <service directory>` where
     :ref:`winss-supervise` has not been started yet.

.. seealso::

   :ref:`winss-svc`
      Can be used to send commands to the :ref:`winss-supervise` process.

.. _winss-svscanctl:

winss-svscanctl.exe
-------------------

:ref:`winss-svscanctl` sends commands to a running :ref:`winss-svscan` process.

.. code-block:: bat

   Usage: winss-svscanctl.exe [options] scandir

   Options:
     --help       Print usage and exit.
     --version    Print the current version of winss and exit.
     -v[<level>], --verbose[=<level>]
                       Sets the verbose level.
     -a,          --alarm
                       Perform a scan of scandir.
     -b,          --abort
                       Close svscan only.
     -n,          --nuke
                       Prune supervision tree.
     -q,          --quit
                       Stop supervised process and svscan.

:ref:`winss-svscanctl` sends the given series of commands to the
:ref:`winss-svscan` process monitoring the *scandir* :term:`scan directory`,
then exits 0. It exits 111 if it cannot send a command, or 100 if no
:ref:`winss-svscan` process is running on *scandir*.

Options
^^^^^^^

 -a\, --alarm
    :ref:`winss-svscan` will immediately perform a scan of *scandir* to check
    for :term:`services <service>`.
 -b\, --abort
    :ref:`winss-svscan` will run into its finishing procedure. It will not kill
    any of the maintained :ref:`winss-supervise` processes.
 -n\, --nuke
    :ref:`winss-svscan` will kill all the :ref:`winss-supervise` processes it
    has launched but that did not match a :term:`service directory` last time
    *scandir* was scanned, i.e. it prunes the supervision tree so that it
    matches exactly what was in *scandir* at the time of the last scan.
    A :kbd:`Control-Break` is sent to the :ref:`winss-supervise` processes
    supervising :term:`services <service>` and also the :ref:`winss-supervise`
    processes supervising loggers.

.. _signal: https://msdn.microsoft.com/en-us/library/windows/desktop/ms682541(v=vs.85).aspx
.. _iso_timestamp: http://en.wikipedia.org/wiki/ISO_8601
