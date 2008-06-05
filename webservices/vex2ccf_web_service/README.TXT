This is the vex2ccf webservice that generates a skeleton of a
ctrl-file from a vex-file. 

Requirements
------------
* I used the ZSI version in "Orig/".
* Python vex parser.

  You will also need the vex-parser compiled with the python
  binding. This uses boost.python.

  The vex-parser can be found in <svnroot>/lib/vex_parser/vex_parser

  You will have to configure it with --enable-pythonlib and put the
  libaries in a convenient place using --prefix (make sure the
  PYTHON_PATH can find the libary)


Scripts
-------

After running "make" you have two python scripts:
 ./service.py
and
 ./request.py

You can start the service without parameters (maybe "nohup service.py"
to let it continue running after you close the terminal).

The request expects one argument: the vex-file. It writes the
ctrl-file to standard output. This can be used for testing.
