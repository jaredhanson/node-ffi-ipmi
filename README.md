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


example
=======

Look at ipmi.js

