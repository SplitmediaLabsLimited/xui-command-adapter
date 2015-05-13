/* globals External */

(function() {
	'use strict';
	
	var websocket = new window.WebSocket('ws://127.0.0.1:7877',
		'lws-mirror-protocol');
	var container = document.getElementById("messages");
	var childNode;
	
	try {
		websocket.onopen = function() {
			console.log('Connected to Socket');
		};
		
		websocket.onmessage = function(msg) {
			if (!msg.data) return;
			
			try {
				var message = JSON.parse(msg.data);
			
				if (/^get/i.test(message.name)) {
					window.External[message.name](function(data) {
						websocket.send(JSON.stringify(data));
					});
				} else if (/^set/i.test(message.name)) {
					window.External[message.name].call(this, message.value);
				}
				
				childNode = document.createElement('p');
				childNode.innerText = message.name;
				
				if (message.value !== undefined)
				{
					childNode.innerText += ', ' + message.value;
				}
				
				container.appendChild(childNode);
			} catch (e) {
				// do nothing for now
			}
		};
		
		websocket.onclose = function() {
			console.log('Connection closed');
		};
		
		// websocket.send('string');
	} catch (e) {
		console.error('Error caught: ' + e);
	}
})();