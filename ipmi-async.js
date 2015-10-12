var ref = require('ref');
var Promise = require('bluebird');
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

function ipmiTool(cmdlist) {
    var intf = libipmi.intfLoad(intfname);
    if (intf.isNull())
        return Promise.reject(new Error("error loading " + intfname + " interface"));
                
    return new Promise(function(resolve, reject) {
        libipmi.intfSessionSetHostname.async(intf, host, function(err, res) {
            if (err) throw err;
            if (0 > res)
                reject("error setting host");
            else
                resolve(intf);                
        });            
    }).then(function(intf) {
        return new Promise(function(resolve, reject) {
            libipmi.intfSessionSetUsername.async(intf, user, function(err, res) {
                if (err) throw err;
                if (0 > res)
                    reject("error setting user");
                else
                    resolve(intf);                
            });            
        });
    }).then(function(intf) {
        return new Promise(function(resolve, reject) {
            libipmi.intfSessionSetPassword.async(intf, password, function(err, res) {
                if (err) throw err;
                if (0 > res)
                    reject("error setting password");
                else
                    resolve(intf);                
            });            
        });
    }).then(function(intf) {
        return new Promise(function(resolve, reject) {
            libipmi.intfSessionSetPrvLvl.async(intf, 0x4, function(err, res) {
                if (err) throw err;
                if (0 > res)
                    reject("error setting privilage level");
                else
                    resolve(intf);                
            });            
        });
    }).then(function(intf) {
        return new Promise(function(resolve, reject) {
            libipmi.intfSessionSetLookupBit.async(intf, 0x10, function(err, res) {
                if (err) throw err;
                if (0 > res)
                    reject("error setting lookup bit");
                else
                    resolve(intf);                
            });            
        });
    }).then(function(intf) {
        return new Promise(function(resolve, reject) {
            libipmi.intfSessionSetCipherSuiteID.async(intf, 3, function(err, res) {
                if (err) throw err;
                if (0 > res)
                    reject("error setting cipher suite id");
                else
                    resolve(intf);                
            });            
        });
    }).then(function(intf) {
        return new Promise(function(resolve, reject) {
            libipmi.intfOpen.async(intf, function(err, res) {
                if (err) throw err;
                if (0 > res)
                    reject("error opening interface " + intfname);
                else
                    resolve(intf);                
            });            
        });
    }).then(function(intf) {
        return new Promise(function(resolve, reject) {
            var argc = cmdlist.length + 1;
            var argv = new Buffer(ref.sizeof.pointer * argc);
            argv.writePointer(new Buffer('libipmi\0'), 0);
            for (var i = 0; i < argc-1; i++) {
	            var str = cmdlist[i] + '\0';
                argv.writePointer(new Buffer(str), ((i+1)*ref.sizeof.pointer));
            }
            
            var buf = ref.alloc(ref.refType(ref.types.char));
            var len = ref.alloc('int', 0);
            libipmi.runCommand.async(intf, argc, argv, buf, len, function(err,status) {
                if (!buf.isNull() && 0 !== len.deref()) {
                    var result = ref.reinterpret(buf.deref(), len.deref());
                    if (0 === status) {
                        console.log('%s', ref.readCString(result));
                        libipmi.freeOutBuf.async(buf.deref(), function() {
                            resolve(intf);
                        });
                    } else {
                        reject(result + " (status=" + status + ")");
                    }
                } else {
                    reject('unexpected result');
                }     
            });
        });
    }).finally(function() {
        libipmi.finishInterface.async(intf, function(err, res) {
            if (err)  
                console.log(err.message);
            if (0 > res)
                console.log("error finishing " + intfname + " interface (status=" + res + ")");
        }); 
    });
}
ipmiTool(["-c", "-v", "sdr"]);
/*
ipmiTool(["-v", "raw", "6", "1"]);
ipmiTool(["-c", "sel", "list", "last", "25"]);
ipmiTool(["sensor"]);
ipmiTool(["chassis", "status"]);
ipmiTool(["chassis", "identify"]);
ipmiTool(["lan", "print"]);
ipmiTool(["fru", "print"]);
*/

