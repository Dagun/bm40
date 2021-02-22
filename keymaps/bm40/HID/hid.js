const vid = 0x4B50;
const pid = 0x3430;
const usagePage = 0xFF60;
const usage = 0x61;

let HID = require('node-hid');
let devices = HID.devices();
let deviceInfo = devices.find(function(d) {
	let isKeyboard = d.vendorId===vid && d.productId===pid;
	return isKeyboard && d.usagePage===usagePage && d.usage===usage;
});

if(deviceInfo) { 
	let device = new HID.HID(deviceInfo.path);
	
	device.on("data",  function(data) {
		console.log("Data:" + data);
		device.write([0x00,1,250,250,250,10]);
	});
	device.on("error", function(err) {
		console.log(err);
	});
	
}