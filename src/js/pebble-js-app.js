/*jslint browser: true, sloppy: true, white: true, vars: true */
/*global Pebble */
//-----------------------------------------------------------------------------
// Lines above are for jslint, the JavaScript verifier.  http://www.jslint.com/
//-----------------------------------------------------------------------------

var CONFIG_OPTIONS = [
	{ name: "blackOnWhite",    type: "boolean" },
	{ name: "largerClockFont", type: "boolean" }
];

(function() {

	// ENTRY POINT
	function showConfiguration() {
		console.log("This is showConfiguration.");

		var url = "http://webonastick.com/watchfaces/dot-matrix-2/config/";
		var q = [];

		var options = {};
		CONFIG_OPTIONS.forEach(function(option) {
			options[option.name] = localStorage.getItem(option.name);
		});

		CONFIG_OPTIONS.forEach(function(option) {
			if (options[option.name] !== null) {
				q.push(option.name + "=" + encodeURIComponent(options[option.name]));
			}
		});
		
		if (q.length) {
			url = url + "?" + q.join("&");
		}

		console.log("showConfiguration(): going to " + JSON.stringify(url));
		Pebble.openURL(url);
	}

	function setConfigFrom(o) {
		console.log("This is setConfigFrom.");
		console.log("    o = " + JSON.stringify(o));

		var message = {}, index = 0;
		CONFIG_OPTIONS.forEach(function(option) {
			localStorage.setItem(option.name, o[option.name]);
			message[index] = o[option.name];
			index += 1;
		});
		Pebble.sendAppMessage(message);
	}

	function configurationClosed(e) {
		console.log("This is configurationClosed.");

		if (e.response && e.response !== "CANCELLED") {
			try {
				var settings = JSON.parse(decodeURIComponent(e.response));
				if (Object.keys(settings).length <= 0) {
					return;
				}
				setConfigFrom(settings);
			} catch (ignore) {
			}
		}
	}

	function appmessage(data) {
		console.log("This is appMessage.");

		try {
			setConfigFrom(data);
		} catch (ignore) {
		}
	}

	function webviewclosed(e) {
		console.log("This is webviewclosed.");

		if (e.response && e.response !== 'CANCELLED') {
			try {
				var settings = JSON.parse(decodeURIComponent(e.response));
				if (Object.keys(settings).length <= 0) {
					return; 
				}
				setConfigFrom(settings);
			} catch (ignore) {
			}
		}
	}

	Pebble.addEventListener("appmessage", appmessage);
	Pebble.addEventListener("showConfiguration", showConfiguration);
	Pebble.addEventListener("configurationClosed", configurationClosed);
	Pebble.addEventListener("webviewclosed", webviewclosed);
}());


