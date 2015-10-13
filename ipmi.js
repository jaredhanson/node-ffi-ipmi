var ref = require('ref');
var libipmi = require('./libipmi');

if (process.argv.length < 6) {
    console.log('Arguments: ' + process.argv[0] + ' ' + process.argv[1] + ' <interface> <host> <user> <passwd>')
    process.exit()
}

var intfname = process.argv[2] || 'lanplus';
var host = process.argv[3] || '172.31.128.1';
var user = process.argv[4] || 'admin';
var password = process.argv[5] || 'admin';

console.log('interface: %s, host: %s, user: %s, password: %s', 
    intfname, host, user, password);

var intf = libipmi.intfLoad(intfname);
if (!intf) {
    console.log('invalid interface %s', intfname);
}
if (0 > libipmi.intfSessionSetHostname(intf, host)) {
    console.log('error setting host %s', host);
}
if (0 > libipmi.intfSessionSetUsername(intf, user)) {
    console.log('error setting user %s', user);
}
if (0 > libipmi.intfSessionSetPassword(intf, password)) {
    console.log('error setting password %s', password);
}
if (0 > libipmi.intfSessionSetPrvLvl(intf, 0x4)) {
    console.log('error setting privilage level');
}
if (0 > libipmi.intfSessionSetLookupBit(intf, 0x10)) {
    console.log('error setting lookup bit');
}
if (0 > libipmi.intfSessionSetSOLEscChar(intf, '~')) {
    console.log('error setting SOL escape character');
}
if (0 > libipmi.intfSessionSetCipherSuiteID(intf, 3)) {
    console.log('error setting cipher suite');
}
if (0 > libipmi.intfOpen(intf)) {
    console.log('error opening %s interface', intfname);
}

/*
 *  runCommand function to build argument list, 
 *  execute the command and log output
 */
function runCommand(intf, cmdlist) {
    /* build up a valid argument list */
    var argc = cmdlist.length + 1;
    var argv = new Buffer(ref.sizeof.pointer * argc);
    argv.writePointer(new Buffer('libipmi\0'), 0);
    for (var i = 0; i < argc-1; i++) {
	    var str = cmdlist[i] + '\0';
        argv.writePointer(new Buffer(str), ((i+1)*ref.sizeof.pointer));
    }

    /* execute the command and handle the output data */
    var buf = ref.alloc(ref.refType(ref.types.char));
    var len = ref.alloc('int', 0);
    var status = libipmi.runCommand(intf, argc, argv, buf, len);
    if (!buf.isNull() && 0 !== len.deref()) {
        var result = ref.reinterpret(buf.deref(), len.deref());
        if (0 === status) {
            console.log('%s', ref.readCString(result));
        } else {
            console.log('%s (status=%d)', result, len.deref());
        }
        libipmi.freeOutBuf(buf.deref());
      
    } else {
        console.log('unexpected result');
    }
}
runCommand(intf, ["-c", "-v", "sdr"]);
/*
runCommand(intf, ["-v", "raw", "6", "1"]);
runCommand(intf, ["-c", "sel", "list", "last", "25"]);

runCommand(intf, ["sensor"]);
runCommand(intf, ["chassis", "status"]);
runCommand(intf, ["chassis", "identify"]);
runCommand(intf, ["lan", "print"]);
runCommand(intf, ["fru", "print"]);
*/

/* always cleanup */ 
libipmi.finishInterface(intf); 
