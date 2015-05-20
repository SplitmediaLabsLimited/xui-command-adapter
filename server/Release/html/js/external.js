(function() {
	'use strict';

	var _callbacks = {};
	
	window.OnAsyncCallback = function(asyncId, res) {
		var callback = _callbacks[asyncId];
		
		if (typeof callback === 'function') {
			callback.call(this, decodeURIComponent(res));
		}
	};
	
	window.External = {
		'getScenes': function(callback) {
			if (typeof callback !== 'function') return false;
			
			if (window.external.AppGetPropertyAsync) {
				var async = window.external.AppGetPropertyAsync('presetconfig');
				
				_callbacks[async] = function(xml) {
					var sceneJSON = window.XML.toJSON(xml);
					var scenes    = [];
					
					sceneJSON = sceneJSON.length > 0 ? sceneJSON[0].children : sceneJSON;
					
					for (var i = 0; i < sceneJSON.length; i++) {
						scenes.push({
							'id'    : i,
							'name'  : sceneJSON[i].name,
							// Crap that, we'll assume we're using the MAIN view
							'viewID' : '0',
							'items'  : sceneJSON[i].children
						});
					}
					
					callback.call(this, scenes);
				};
			}
		},
		'setScene': function(scene) {
			if(scene === undefined) return;
			
			// Not sure if I need async or not, but I'll just use it :)
			scene = parseInt(scene) + 1;
			scene = scene < 1 ? 1 : scene;
			scene = scene > 12 ? 12 : scene;
			
			if (window.external.AppSetPropertyAsync) {
				window.external.AppSetPropertyAsync('preset', String(scene));
			}
		}
	};
})();