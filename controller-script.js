document.querySelector('.no-js-error').remove();
var Encoder = new TextEncoder(); // always utf-8

let UUIDs = [];
let BTControllerData = {};
let BTCharacteristics = {};

// First lets fill the controller data.
fetch('./controller-data.json', {mode: 'no-cors'}).then((response)=>{
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
});

// The BT needs to be initiated by a user action, we use a needlely long button for that.
document.querySelector('button#enableBT').addEventListener('pointerup', ()=>{
	
	// Connect to the device in the JSO and pass all the UUIDs we want to accept since were going full custom
	navigator.bluetooth.requestDevice({
		filters: [{name:BTControllerData._name}],
		optionalServices: UUIDs
	
	// Connect to the BT GATT server
	}).then((device)=>{
		console.log(`Connected to ${device.name}.`);
		return device.gatt.connect();
	
	// Loop through the Controller Data
	}).then((server)=>{
		Object.keys(BTControllerData).forEach((serviceName,serviceIndex)=>{

			// If the service starts with _ it's not a real service, skill it and add a null to the services array in it's place
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
				});
			}
		});

	}).catch((error)=>{
		console.log(error);
	});
});

// handle updating the values when the inputs are changed
const inputs = document.querySelectorAll('input');
for(input of inputs){
	input.addEventListener('change',(e)=>{
		let characteristic = e.target.id;
		let value          = e.target.value;

		// And the actual update
		BTCharacteristics[characteristic].writeValue(Encoder.encode(value))
		console.log(characteristic,value)
	});
};