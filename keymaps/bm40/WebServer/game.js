const express = require('express');
const bodyParser = require('body-parser');
const app = express();
const port = 3333;

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
	
	// device.on("data",  function(data) {
	// 	console.log("Data:" + data);
	// 	device.write([0x00,1,250,250,250,10]);
	// });
	// device.on("error", function(err) {
	// 	console.log(err);
	// });

    app.use(bodyParser.json());

    app.post('/reveal',(req, res) =>{
        device.write([0x00,3,req.body.row,req.body.col]);
        console.log(req.body.row);
        console.log(req.body.col);
        res.send('Hello World, from express');
    });

    app.post('/flag',(req, res) =>{
        console.log(req.body);
        device.write([0x00,4,req.body.row,req.body.col]);
        res.send('Hello World, from express');
    });

    app.get('/reset',(req, res) =>{
        console.log("reset");
        device.write([0x00,2]);
        res.send('Hello World, from express');
    });

    app.get('/inc',(req, res) =>{
        device.write([0x00,5]);
        console.log("has been inc");
        res.send('');
    });

    app.get('/dec',(req, res) =>{
        device.write([0x00,6]);
        console.log("has been dec");
        res.send('');
    });

    app.get('/left',(req, res) =>{
        device.write([0x00,7]);
        res.send('');
    });

    app.get('/down',(req, res) =>{
        device.write([0x00,8]);
        res.send('');
    });

    app.get('/up',(req, res) =>{
        device.write([0x00,9]);
        res.send('');
    });

    app.get('/right',(req, res) =>{
        device.write([0x00,10]);
        res.send('');
    });

    app.get('/snake',(req, res) =>{
        device.write([0x00,11]);
        res.send('');
    });

    app.get('/minesweepers',(req, res) =>{
        device.write([0x00,12]);
        res.send('');
    });

    app.use(express.static("game"));

    app.listen(port, () => console.log(`Hello world app listening on port ${port}!`));
        
}