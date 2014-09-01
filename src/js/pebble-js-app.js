/*jslint browser: true, sloppy: true, white: true, vars: true */
/*global Pebble */
//-----------------------------------------------------------------------------
// Lines above are for jslint, the JavaScript verifier.  http://www.jslint.com/
//-----------------------------------------------------------------------------

(function() {

	// ENTRY POINT
	function showConfiguration() {
		console.log("This is showConfiguration.");

		var url = "http://webonastick.com/watchfaces/dot-matrix-1/config/";
		var q = [];
		
		var blackOnWhite = localStorage.getItem("blackOnWhite");
		var showDate     = localStorage.getItem("showDate");
		var showBattery  = localStorage.getItem("showBattery");

		console.log("showConfiguration(): from localStorage, blackOnWhite = " + JSON.stringify(blackOnWhite));
		console.log("showConfiguration(): from localStorage, showDate = " + JSON.stringify(showDate));
		console.log("showConfiguration(): from localStorage, showBattery = " + JSON.stringify(showBattery));

		if (blackOnWhite !== null) {
			q.push("blackOnWhite=" + encodeURIComponent(blackOnWhite));
		}
		if (showDate !== null) {
			q.push("showDate=" + encodeURIComponent(showDate));
		}
		if (showBattery !== null) {
			q.push("showBattery=" + encodeURIComponent(showBattery));
		}
		if (q.length) {
			url = url + "?" + q.join("&");
		}

		console.log("showConfiguration(): going to " + JSON.stringify(url));
		Pebble.openURL(url);
	}

	function setConfigFrom(o) {
		console.log("This is setConfigFrom.");
		console.log("    o = " + JSON.stringify(o));

		localStorage.setItem("blackOnWhite", o.blackOnWhite);
		localStorage.setItem("showDate",     o.showDate);
		localStorage.setItem("showBattery",  o.showBattery);
		Pebble.sendAppMessage({
			0: o.blackOnWhite,
			1: o.showDate,
			2: o.showBattery
		});
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


