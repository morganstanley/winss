.. _service-dir:

=================
Service Directory
=================

A :term:`service directory` may contain the following:

.. _run:

run
---
A file `run`_ which contains the executable and the arguments to run the
:term:`service`. It is read and the process described in the contents are
started every time the :term:`service` must be started.

.. note::

   - The startup location for the executable will be the service
     directory itself.

   - Environment variable substitution will be performed on the process
     described by `run`_ before executing. Additional environment variables
     can be created as mentioned below.

.. _finish:

finish
------
An optional `finish`_ file which will run after the `run`_ process has
finished. By default, the `finish`_ process must do its work and exit in
less than 5 seconds; if it takes more than that, it is killed. The maximum
duration of a `finish`_ execution can be configured via the 
`timeout-finish`_ file mentioned below.

.. note::

   Similarly with `run`_, environment variables will be substituted with the
   addition of :envvar:`SUPERVISE_RUN_EXIT_CODE` which has the exit code of the
   `run`_ process. Additional environment variables mentioned below will
   also be available to the `finish`_ process.

.. _supervise:

supervise
---------
A directory named `supervise`_, which is automatically created by
:ref:`winss-supervise` to store its information. This directory must be
writable.

.. _down:

down
----
An optional, empty file `down`_, which if exists will make the default
state *down* and not *up* such that when :ref:`winss-supervise` starts
then the `run`_ process will not be started until signaled using
:ref:`winss-svc` -u.

.. _timeout-finish:

timeout-finish
--------------
An optional file `timeout-finish`_ which contains an unsigned integer that
is the maximum number of milliseconds after which the `finish`_ process
can execute for. It will be terminated after this period has expired.
A value of 0 allows the `finish`_ process to run forever.

.. _env:

env
---
An optional directory named `env`_ which contains files that represent the
environment variable names and their contents are the values of the
environment variables. For example a file named **env/USER** with the contents
**foo** would be ``USER=foo`` when running a process. Like the contents of
`run`_ and `finish`_, the values can be substituted with current environment
variables. Using this you can append to the **PATH** rather than overwriting
it.

.. _log:

log
---
An optional *service directory* named `log`_, which if exists and
:ref:`winss-supervise` is started by :ref:`winss-svscan`, then the
:ref:`winss-svscan` process will start an additional :ref:`winss-supervise`
on the `log`_ *service directory* with the standard input piped from the
standard output of the former :ref:`winss-supervise` process. For example
if the service **foo** has a `log`_ folder then both **foo** and **foo/log**
will be supervised and the output of **foo** will be sent to the input of
**foo/log**. Typically :ref:`winss-log` can be used to log although not
necessarily.

.. warning::

  There may be additional files/directories which will be used by
  :ref:`winss-supervise` in the future. The only file/directory which
  is guaranteed never to be used by :ref:`winss-supervise` is **data**.
  It is therefore recommended that any specific application data in that
  file/directory.
