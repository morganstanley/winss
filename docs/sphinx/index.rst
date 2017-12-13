.. toctree::
   :caption: Fundamentals
   :maxdepth: 2
   :hidden:

   service_dir
   components

=========================
Windows Supervision Suite
=========================

.. include:: ./include/about.rst

Definitions
-----------

.. important::

   .. include:: ./include/glossary.rst

Components
----------

- :ref:`winss-supervise`: monitors a long-lived process/service.
- :ref:`winss-svscan`: starts and monitors a collection of
  :ref:`winss-supervise` processes.
- :ref:`winss-log`: reliable logging program with automated log rotation.
- :ref:`winss-svc`: sends commands to a running :ref:`winss-supervise` process.
- :ref:`winss-svok`: checks whether a service directory is currently supervised
- :ref:`winss-svstat`: prints a short, human-readable summary of the state of
  the process monitored by :ref:`winss-supervise`.
- :ref:`winss-svwait`: blocks until a collection of :ref:`winss-supervise`
  processes goes up, or down.
- :ref:`winss-svscanctl`: sends commands to a running :ref:`winss-svscan`
  process.

.. include:: ./include/lib.rst
