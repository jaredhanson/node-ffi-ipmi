'use strict';

var ref = require('ref');
var ffi = require('ffi');

var intPtr = ref.refType('int');
var charPtr = ref.refType(ref.types.char)
var charPtrPtr = ref.refType(charPtr);

var libipmi = ffi.Library(__dirname + '/libipmi', {
    'intfLoad': ['pointer', ['string']],
    'intfOpen': ['int', ['pointer']],
    'freeOutBuf': ['void', [charPtr]],
    'createOutBuf': [charPtr, ['int']],
    'intfSessionSetHostname': ['int',['pointer','string']],
    'intfSessionSetUsername': ['int',['pointer','string']],
    'intfSessionSetPassword': ['int',['pointer','string']],
    'intfSessionSetPrvLvl': ['int',['pointer','int']],
    'intfSessionSetLookupBit': ['int',['pointer','int']],
    'intfSessionSetCipherSuiteID': ['int',['pointer','int']],
    'intfSessionSetSOLEscChar': ['int',['pointer','char']],
    'intfSessionSetTimeout': ['int',['pointer','int']],
    'getUserName': ['int',['pointer','int', charPtr]],
    'finishInterface': ['int', ['pointer']],
    'runCommand' : ['int', ['pointer', 'int', 'pointer', charPtrPtr, intPtr]]
});
module.exports = libipmi;
