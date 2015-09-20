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
    'finish_interface': ['int', ['pointer']],
    'run_command' : ['string', ['pointer', 'int', 'pointer']]
})
module.exports = Object.create(libipmi);
