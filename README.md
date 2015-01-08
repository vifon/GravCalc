GravCalc
========

Accelerometer-based calculator for the Pebble smartwatch.

Note: this application is not yet available on the Pebble appstore.

Run `make doc` to generate the documentation with Doxygen.

USAGE
-----

Buttons:  
- **lower**: click  
- **middle**: push to the stack  
- **middle longpress**: empty the stack  
- **upper**: backspace / delete the last number from the stack  
- **upper longpress**: delete the current number

The calculator uses the
[Reverse Polish Notation (RPN)](http://en.wikipedia.org/wiki/Reverse_Polish_notation).

INSTALLATION
------------

    $ make
    $ export PEBBLE_PHONE=192.168.???.???   # (your phone's IP)
    $ make install

COPYRIGHT
---------

Copyright (C) 2015 Wojciech Siewierski <wojciech dot siewierski at onet dot pl>

Author: Wojciech Siewierski <wojciech dot siewierski at onet dot pl>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
