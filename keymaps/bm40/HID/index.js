const HID = require('node-hid');
const process = require('process');
const ConsoleController = require('./ConsoleController');
const AnimationController = require('./AnimationController');
const devices = HID.devices();
const vid = 0x7432;
const pid = 0x0658;
const usagePage = 0xFF60;
const usage = 0x61;

const KeyboardController = require('./KeyboardController');
const CSGOController = require('./CSGOController');

const keyboardInfo = devices.find( d => d.vendorId === vid && d.productId === pid && d.usage == usage && d.usagePage === usagePage);
if(!keyboardInfo) {
    console.log('Keyboard not found!');
    process.exit();
}
const keyboard = new HID.HID(keyboardInfo.path);

const keyboardController = new KeyboardController(keyboard);
const animationController = new AnimationController(keyboardController);
const csc = new CSGOController(keyboardController, animationController);
csc.start();


process.on('SIGINT', function() {
    console.log('Closing connection');
    keyboard.close();
    process.exit();
});