/*jslint browser: true, sloppy: true */
//-----------------------------------------------------------------------------
// Lines above are for jslint, the JavaScript verifier.  http://www.jslint.com/
//-----------------------------------------------------------------------------

function console_log() {
	var $ = jQuery;
	var i;
	for (i = 0; i < arguments.length; i += 1) {
		$("#console_log").append($("<div></div>").text(arguments[i]));
	}
}

console_log("This is config.js.  location.href = " + JSON.stringify(location.href));

(function($) {
	console_log("This is an anonymous function executed via (function($) { ... }(jQuery)).");
	$(document).ready(function($) {
		console_log("This is an anonymous function, executed via $(document).ready(...).");
	});
}(jQuery));

jQuery(function($) {
	console_log("This is config.js's ready handler, executed via jQuery(function($) { ... }).");

	var $form = $("form.configForm");
	var $cb_blackOnWhite = $form.find(":checkbox[name='blackOnWhite']");
	var $cb_showDate     = $form.find(":checkbox[name='showDate']");
	var $cb_showBattery  = $form.find(":checkbox[name='showBattery']");
	var $b_cancel = $form.find("#b-cancel");
	var $b_submit = $form.find("#b-submit");
	function saveOptions() {
		return {
			blackOnWhite: $cb_blackOnWhite.is(":checked") ? true : false,
			showDate:     $cb_showDate    .is(":checked") ? true : false,
			showBattery:  $cb_showBattery .is(":checked") ? true : false
		};
	}
	function param(name) {
		var rx = new RegExp("(?:^|\\?|\\&)" + name + "=(.*?)(?:$|\\&)");
		var match = rx.exec(location.search);
		console_log("config.js param(): location.search = " + JSON.stringify(location.search));
		console_log("config.js param(): rx = " + JSON.stringify(rx));
		if (match) {
			console_log("config.js param(): match = " + JSON.stringify(match));
			return match[1];
		} else {
			console_log("config.js param(): no match.");
			return null;
		}
	}

	$b_cancel.click(function() {
		document.location = "pebblejs://close";
	});
	$b_submit.click(function() {
		document.location = "pebblejs://close#" + encodeURIComponent(JSON.stringify(saveOptions()));
	});
	
	var blackOnWhite = param("blackOnWhite");
	var showDate     = param("showDate");
	var showBattery  = param("showBattery");

	console_log("config.js: blackOnWhite = " + JSON.stringify(blackOnWhite));
	console_log("config.js: showDate     = " + JSON.stringify(showDate));
	console_log("config.js: showBattery  = " + JSON.stringify(showBattery));
	
	if (Number(blackOnWhite) || blackOnWhite === "true") {
		console_log("A");
		$cb_blackOnWhite.attr("checked", true).checkboxradio("refresh");
	}
	if (Number(showDate) || showDate === "true") {
		console_log("B");
		$cb_showDate.attr("checked", true).checkboxradio("refresh");
	}
	if (Number(showBattery) || showBattery === "true") {
		console_log("C");
		$cb_showBattery.attr("checked", true).checkboxradio("refresh");
	}
});
