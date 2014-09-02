/*jslint browser: true, sloppy: true, white: true, vars: true */
//-----------------------------------------------------------------------------
// Lines above are for jslint, the JavaScript verifier.  http://www.jslint.com/
//-----------------------------------------------------------------------------
/*global jQuery */

function console_log() {
	var $ = jQuery;
	var i;
	for (i = 0; i < arguments.length; i += 1) {
		$("#console_log").append($("<div></div>").text(arguments[i]));
	}
}

var CONFIG_OPTIONS = [
	{ name: "blackOnWhite",    type: "boolean" },
	{ name: "showDate",        type: "boolean" },
	{ name: "showBattery",     type: "boolean" },
	{ name: "largerClockFont", type: "boolean" }
];

jQuery(function($) {
	//console_log("This is config.js's ready handler, executed via jQuery(function($) { ... }).");

	var $form = $("form.configForm");

	var $cb = {};
	CONFIG_OPTIONS.forEach(function(option) {
		if (option.type === "boolean") {
			$cb[option.name] = $form.find(":checkbox[name='" + option.name + "']");
		}
	});

	var $b_cancel = $form.find("#b-cancel");
	var $b_submit = $form.find("#b-submit");

	function saveOptions() {
		var result = {};
		CONFIG_OPTIONS.forEach(function(option) {
			result[option.name] = $cb[option.name].is(":checked") ? true : false;
		});
		//console_log("saveOptions(): result = " + JSON.stringify(result));
		return result;
	}

	function param(name) {
		var rx = new RegExp("(?:^|\\?|\\&)" + name + "=(.*?)(?:$|\\&)");
		var match = rx.exec(location.search);
		//console_log("config.js param(): location.search = " + JSON.stringify(location.search));
		//console_log("config.js param(): rx = " + JSON.stringify(rx));
		if (match) {
			//console_log("config.js param(): match = " + JSON.stringify(match));
			return match[1];
		}
		//console_log("config.js param(): no match.");
		return null;
	}

	$b_cancel.click(function() {
		document.location = "pebblejs://close";
	});
	$b_submit.click(function() {
		document.location = "pebblejs://close#" + encodeURIComponent(JSON.stringify(saveOptions()));
	});

	var options = {};

	CONFIG_OPTIONS.forEach(function(option) {
		options[option.name] = param(option.name);
		if (option.type === "boolean") {
			if (Number(options[option.name]) || options[option.name] === "true") {
				$cb[option.name].attr("checked", true).checkboxradio("refresh");
			}
		}
	});
});
