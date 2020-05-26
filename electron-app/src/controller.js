let prepared = false;
let connected = false;
let connecting = false;

var Encoder = new TextEncoder(); // always utf-8
var BluetoothDevice;
let UUIDs = [];
let BTControllerData = {};
let BTCharacteristics = {};
// let prepared = false;
let ready = false;
let connectionErrors = 0;

/*
	Before we can even consider playing with bluetooth we need to prepare some data
	The connect function will not work till this has completed.
*/
function prepare(){
	fetch('./data.json', {mode: 'no-cors'}).then((response)=>{
		return response.json();
	}).then((json)=>{
		BTControllerData = json;
		// We need to let it know what additional UUIDs we want to support, in thsi case they are all custom so we need them all
		Object.keys(BTControllerData).forEach((k)=>{
			if(BTControllerData[k].UUID!=undefined){
				UUIDs.push(BTControllerData[k].UUID);
				BTControllerData[k].characteristics.forEach((characteristic)=>{
					UUIDs.push(Object.values(characteristic)[0]);
				});
			}
		});
		prepared = true;
	});
}
prepare();

/*
	If we are prepared and not already connected we can attempt to connect
*/
function connect(){
	if(!prepared){
		alert('Still waiting for preperations to complete.')
		return false;
	}
	if(connected){
		alert('Connection already ready already.')
		return false;
	}

	// Connect to the device in the JSO and pass all the UUIDs we want to accept since were going full custom
	connecting = true;
	BluetoothDevice = navigator.bluetooth.requestDevice({
		filters: [{name:BTControllerData._name}],
		optionalServices: UUIDs
	
	// Connect to the BT GATT server
	}).then((device)=>{
		device.addEventListener('gattserverdisconnected',()=>{
			ready = false;
			connect();
		});
		console.log(`Connected to ${device.name}.`);
		return device.gatt.connect();
	
	// Loop through the Controller Data
	}).then((server)=>{
		Object.keys(BTControllerData).forEach((serviceName,serviceIndex)=>{

			// If the service starts with _ it's not a real service, kill it and add a null to the services array in it's place
			if(serviceName.substr(0,1)!="_"){
				// Get the service from the server, then loop through it's characteristics
				server.getPrimaryService(BTControllerData[serviceName].UUID).then((service)=>{
					BTControllerData[serviceName].characteristics.forEach((characteristic)=>{
						
						// Get the characteristic by UUID and add it to that array.
						let charUUID = Object.values(characteristic)[0];
						service.getCharacteristic(charUUID).then((char)=>{
							BTCharacteristics[serviceName+':'+Object.keys(characteristic)[0]] = char
						});
					});
				}).then(()=>{
					connected = true;
					connecting = false;
				});
			}
		});

	}).catch((error)=>{
		console.log(error);
	});
}

// The BT needs to be initiated by a user action, we use a needlely long button for that.
document.addEventListener('pointerdown', (e)=>{
	if(!connected && !connecting){
		connect();
	}
});

/*
	Using a single fancy color picker for a more interesting and less annoying interface.
*/
var colorPicker = new iro.ColorPicker("#picker", {
  width: 275,
  colors: ["#FFBB55","#FFBB55","#FFBB55"],
  wheelLightness: false,
  layout: [
  	{component: iro.ui.Wheel },
  	{component: iro.ui.Slider ,	options: { sliderType: 'saturation' }},
  	{component: iro.ui.Slider ,	options: { sliderType: 'value' }},
  ]
});
// This updates the selection box when you play with the wheel
colorPicker.on('color:init',updateColorSelector)
colorPicker.on('color:change',updateColorSelector)
function updateColorSelector(e){
	if(connected){

		// sync the bg color and the input
		document.querySelector('[data-selectorIndex="'+(e.index%3)+'"]').closest('fieldset').style.backgroundColor = e.hexString
		document.querySelector('[data-selectorIndex="'+(e.index%3)+'"]').value = e.hexString;

		// And the actual update
		let characteristic = document.querySelector('[data-selectorIndex="'+(e.index%3)+'"]').id;
		let value = e.hexString;
		BTCharacteristics[characteristic].writeValue(Encoder.encode(value))

		// Sometimes we run thiuswithout changing activation (for color sync)
		if(e.dontActivate===undefined){

			// Set the fieldsets background and change the active one as needed
			document.querySelector('.lights .active').classList.remove('active')
			document.querySelector('[data-selectorIndex="'+(e.index%3)+'"]').closest('fieldset').classList.add('active')

			if(document.querySelector('input#sync').checked){
				for(var i=0; i<3; i++){
					if(e.index!=i){
						let fakeEvent = {
							index: i,
							hexString: value,
							dontActivate: true
						}
						updateColorSelector(fakeEvent)
					}
				}
			}
		}

	}
}
// this updates which wheel selector your using when you click the box.
document.querySelectorAll('fieldset').forEach((field)=>{
	field.addEventListener('click',(e)=>{
		let colorIndex = e.target.querySelector('input').dataset.selectorindex;
		colorPicker.setActiveColor(colorIndex);
		updateColorSelector({index:colorIndex});
	})
});

/*
	The power toggle is the only one not covered by the color picker
*/
document.querySelector('#power input').addEventListener('change',(e)=>{
	if(connected){
		let characteristic = e.target.id;
		let value = e.target.checked ? "On" : "Off";
		BTCharacteristics[characteristic].writeValue(Encoder.encode(value))
	}else{
		e.target.checked = !e.target.checked
		e.stopPropagation();
		e.preventDefault();
		return false
	}
});


/*
	Older HTML input type handlers
*/
// Hangle the color inputs
for(input of document.querySelectorAll('input[type="color"]')){
	input.addEventListener('change',(e)=>{
		if(connected){
			let characteristic = e.target.id;
			let value          = e.target.value;
			// And the actual update
			BTCharacteristics[characteristic].writeValue(Encoder.encode(value))
		}
	});

}
// Hangle the brightness inputs
for(input of document.querySelectorAll('input[type="range"]')){
	input.dataset.lastValue = input.value;
	input.addEventListener('mousemove',(e)=>{
		if(connected){
			if(input.dataset.lastValue != e.target.value){
				let characteristic = e.target.id;
				let value          = e.target.value;
				// And the actual update
				BTCharacteristics[characteristic].writeValue(Encoder.encode(value))
				input.dataset.lastValue = value;
			}
		}
	});

}

