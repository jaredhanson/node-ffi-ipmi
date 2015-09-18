Based on project https://github.com/jaredhanson/node-ffi-ipmi

To Build:

Run ```npm install```

Run ```make```

Run ```nodejs ipmi.js <host> <user> <password>```

See ipmi.js for example how to use FFI to reference libipmi.so

node-ffi-ipmi
=============

wrapping various ipmi related tools and libs for node via node-ffi @ https://github.com/rbranson/node-ffi.git

First, clone https://github.com/speedops/ipmitool.git and configure and make it.

Then, make the ipmi stuff here to do ipmitool stuff in node.js

why
===

i looked at different ways of interfacing ipmitool from node.js.  The options i played with are: 

* http://github.com/joyent/v8plus  -- this has funny documentation (see FAQ).  I had some issues with it, I'm too afraid to talk about this in public
* https://github.com/jmendeth/v8u -- this looked promising.  still too complex
* https://github.com/rbranson/node-ffi -- this is the one I ended up using. Because I can make sense of it.



what
====

* wanted to avoid shelling out to call ipmitool CLI commands
* want to avoid having to connect for every command; want to run multiple commands per established channel
* want a simple way to call -- without having to hack inside ipmitool too much and figure out which sequence of calls to make -- ipmitool does not seem to be designed to be used as a library
* do not want fork/exec or shelling out per command
* do not want hundreds of boilerplate wrappers for each command combo 
* make it fast and be async compatible
* if you understand  ipmitool command or/and if the ipmitool command works for you this wrapper should work for you as well without having to figure out which C functions to call in which order
* it does not prevent you from cooking up more javascripty / nicer API (see ipmi.c and ipmi.js for alternative API that are less stringy).

example
=======

Look at ipmi.js

