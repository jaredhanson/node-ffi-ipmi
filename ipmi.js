var ref = require('ref');
var ffi = require('../node-ffi');

var charPtr = ref.refType(ref.types.char)
var libipmi = ffi.Library('./libipmi', {
    'intf_load': ['pointer', ['string']],
    'intf_session_set_hostname': ['int',['pointer','string']],
    'intf_session_set_username': ['int',['pointer','string']],
    'intf_session_set_password': ['int',['pointer','string']],
    'chassis_power_status': ['int',['pointer']],
    'get_user_name': ['int',['pointer','int', charPtr]],
    'finish_interface': [ 'int', ['pointer']],
    'run_command' : [ 'int', [ 'pointer', 'int', 'pointer']]
})

if (process.argv.length < 5) {
    console.log('Arguments: ' + process.argv[0] + ' ' + process.argv[1] + ' <host> <user> <passwd>')
    process.exit()
}

var host = process.argv[2] || '172.31.128.1';
var user = process.argv[3] || 'admin';
var password = process.argv[4] || 'admin';
console.log('host: %s, user: %s, password: %s', host, user, password);

// TODO: test lanplus - debug segfault
var intf = libipmi.intf_load('lan');
var result = libipmi.intf_session_set_hostname(intf, host);
result = libipmi.intf_session_set_username(intf, user);
result = libipmi.intf_session_set_password(intf,password);

function run_command_string(intf, cmdlist) {
    var argc = cmdlist.length;
    var argv = new Buffer(ref.sizeof.pointer * argc);
    for (var i = 0; i < argc; i++) {
       argv.writePointer(new Buffer(cmdlist[i]), i * ref.sizeof.pointer);
    }
    libipmi.run_command(intf, argc, argv);
}

run_command_string(intf, ["raw", "6", "1"]);
run_command_string(intf, ["chassis", "status"]);
run_command_string(intf, ["chassis", "identify"]);
run_command_string(intf, ["lan", "print"]);
run_command_string(intf, ["sensor"]);
run_command_string(intf, ["fru", "print"]);
libipmi.finish_interface(intf);
