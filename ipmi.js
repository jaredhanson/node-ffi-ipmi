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
    'test_argv' : [ 'int', [ 'pointer', 'int', 'pointer']]
})

console.log('start ipmi test');

var intf0 = libipmi.intf_load('lan');
// lanplus fails with error regarding admin priv.
console.log('intf_load returns : ' + intf0);

var hostName = process.argv[2] || '192.168.0.10';
console.log('using ipmi host ' , hostName);

var userName = process.argv[3] || 'ADMIN';
console.log('using user name ' , userName);

var password = process.argv[4] || 'ADMIN';
console.log('using password ' , password);


var result = libipmi.intf_session_set_hostname(intf0, hostName);
console.log('intf_session_set_hostname ' + result);

result = libipmi.intf_session_set_username(intf0, userName);
console.log('intf_session_set_username ' + result);

result = libipmi.intf_session_set_password(intf0,password);

console.log('intf_session_set_password ' + result);

result = libipmi.chassis_power_status(intf0);

console.log('chassis power status : ' + result);

var buf = new Buffer(256);
result = libipmi.get_user_name(intf0, 2, buf);

console.log('get user name for id 2 : <%s>' , buf.toString('utf8',0,result))

// test passing argv[] via node FFI to C

var argv = new Buffer(ref.sizeof.pointer * 3);
var a = new Buffer('hello');
var b = new Buffer('world');
var c = new Buffer('1');

argv.writePointer(a, 0 * ref.sizeof.pointer);
argv.writePointer(b, 1 * ref.sizeof.pointer);
argv.writePointer(c, 2 * ref.sizeof.pointer);

result = libipmi.test_argv(intf0, 3, argv);


console.log('end ipmi test');
