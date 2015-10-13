var ref = require('ref');
var Promise = require('bluebird');
var libipmi = require('./libipmi');

if (process.argv.length < 6) {
    console.log('Arguments: ' + process.argv[0] + ' ' + process.argv[1] + ' <interface> <host> <user> <passwd>')
    process.exit()
}

function ipmiTool(cmdlist) {

    var intfname = process.argv[2] || 'lanplus';
    var host = process.argv[3] || '172.31.128.1';
    var user = process.argv[4] || 'admin';
    var password = process.argv[5] || 'admin';

    console.log('interface: %s, host: %s, user: %s, password: %s', 
        intfname, host, user, password);
        
    if (intfname === "lan" || intfname === "lanplus" ) {
        if (!user || !password) {
            return Promise.reject(new Error("missing parameters for " + intfname + "interface"));
        }
    }
    if (!cmdlist.length) {
        return Promise.reject(new Error("missing required command parameter"));
    }
    
    var intf = libipmi.intfLoad(intfname);
    if (intf.isNull()) {
        return Promise.reject("error loading " + intfname + " interface");
    }

    return new Promise(function(resolve, reject) {  
        return Promise.fromNode(libipmi.intfSessionSetHostname.async
        .bind(libipmi, intf, host)).then(function(res) {
            if (0 > res) {
                throw new Error("error setting host");
            }
        }).then(function() {
            return Promise.fromNode(libipmi.intfSessionSetUsername.async
            .bind(libipmi, intf, user)).then(function(res) {
                if (0 > res) {
                    throw new Error("error setting user");
                }
            });            
        }).then(function() {
            return Promise.fromNode(libipmi.intfSessionSetPassword.async
            .bind(libipmi, intf, password)).then(function(res) {
                if (0 > res) {
                    throw new Error("error setting password");
                }
            });            
        }).then(function() {
            return Promise.fromNode(libipmi.intfSessionSetPrvLvl.async
            .bind(libipmi, intf, 0x4)).then(function(res) {
                if (0 > res) {
                    throw new Error("error setting privilage level");
                }
            });            
        }).then(function() {
            return Promise.fromNode(libipmi.intfSessionSetLookupBit.async
            .bind(libipmi, intf, 0x10)).then(function(res) {
                if (0 > res) {
                    throw new Error("error setting lookup bit");
                }
            });            
        }).then(function() {
            return Promise.fromNode(libipmi.intfSessionSetCipherSuiteID.async
            .bind(libipmi, intf, 3)).then(function(res) {
                if (0 > res) {
                    throw new Error("error setting cipher suite id");
                }
            });            
        }).then(function() {
            return Promise.fromNode(libipmi.intfOpen.async
            .bind(libipmi, intf)).then(function(res) {
                if (0 > res) {
                    throw new Error("error opening interface " + intfname);
                }
            });            
        }).then(function() {
            var argc = cmdlist.length + 1;
            var argv = new Buffer(ref.sizeof.pointer * argc);
            argv.writePointer(new Buffer('libipmi\0'), 0);
            for (var i = 0; i < argc-1; i++) {
                var str = cmdlist[i] + '\0';
                argv.writePointer(new Buffer(str), ((i+1)*ref.sizeof.pointer));
            }
            
            var buf = ref.alloc(ref.refType(ref.types.char));
            var len = ref.alloc('int', 0);
            return Promise.fromNode(libipmi.runCommand.async
            .bind(libipmi, intf, argc, argv, buf, len)).then(function(status) {
                if (!buf.isNull() && 0 !== len.deref()) {
                    var result = ref.readCString(ref.reinterpret(buf.deref(), len.deref()));
                    if (0 === status) {
                        console.log(result);
                        resolve(result);
                    } else {
                        reject(result);
                    }
                    libipmi.freeOutBuf(buf.deref());
                } else {
                    reject('unexpected ipmi command result');
                }     
            });
        }).catch(function(err) {
            console.log(err.message);
            reject(err);
        }).finally(function() {
            /* This should never fail if intfLoad() succeeded, 
               otherwise we would never get here */
            libipmi.finishInterface(intf);
        });
    });
}
ipmiTool(["sensor"]);
/*
ipmiTool(["-c", "-v", "sdr"]);
ipmiTool(["-v", "raw", "6", "1"]);
ipmiTool(["-c", "sel", "list", "last", "25"]);

ipmiTool(["chassis", "status"]);
ipmiTool(["chassis", "identify"]);
ipmiTool(["lan", "print"]);
ipmiTool(["fru", "print"]);
*/

