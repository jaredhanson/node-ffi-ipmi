var ref = require('ref');
// if node-ffi is install elsewhere , change the following
var ffi = require('../node-ffi');

var charPtr = ref.refType(ref.types.char)

// some basic testing stuff

var libipmi = ffi.Library('./libipmi', {
    'intf_load': ['pointer', ['string']],
    'intf_session_set_hostname': ['int',['pointer','string']],
    'intf_session_set_username': ['int',['pointer','string']],
    'intf_session_set_password': ['int',['pointer','string']],
    'chassis_power_status': ['int',['pointer']],
    'get_user_name': ['int',['pointer','int', charPtr]],
    'test_argv' : [ 'int', [ 'pointer', 'int', 'pointer']],
    'start_interface': [ 'pointer', [ 'string','string','string','string']],
    'finish_interface': [ 'int', ['pointer']],
    'run_command_argv' : [ 'int', [ 'pointer', 'int', 'pointer']]
})

console.log('==== start ipmi test ====');

var hostName = process.argv[2] || '192.168.0.10';
var userName = process.argv[3] || 'ADMIN';
var password = process.argv[4] || 'ADMIN';

console.log('using ipmi host ' , hostName);

console.log('using user name ' , userName);

console.log('using password ' , password);

/*
var intf0 = libipmi.intf_load('lan');

// lanplus fails with error regarding admin priv.
console.log('intf_load returns : ' + intf0);



var result = libipmi.intf_session_set_hostname(intf0, hostName);
console.log('intf_session_set_hostname ' + result);

result = libipmi.intf_session_set_username(intf0, userName);
console.log('intf_session_set_username ' + result);

result = libipmi.intf_session_set_password(intf0,password);

console.log('intf_session_set_password ' + result);
*/



var intf0 = libipmi.start_interface('lan',hostName, userName, password);

result = libipmi.chassis_power_status(intf0);

console.log('chassis power status : ' + result);


var buf = new Buffer(256);
result = libipmi.get_user_name(intf0, 2, buf);

console.log('get user name for id 2 : <%s>' , buf.toString('utf8',0,result))



function  run_command_string(intf, cmd_str) {
    var cmd_arr = cmd_str.split(' ');
    var argv = new Buffer(ref.sizeof.pointer * cmd_arr.length);
    
    for (var i = 0; i < cmd_arr.length; i++) {
	argv.writePointer(new Buffer(cmd_arr[i]), i * ref.sizeof.pointer);
    }

    libipmi.run_command_argv(intf, cmd_arr.length, argv);
}

run_command_string(intf0, "user list");
run_command_string(intf0, "chassis status");

libipmi.finish_interface(intf0);

console.log('==== end ipmi test ===');
