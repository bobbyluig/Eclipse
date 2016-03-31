// Default settings.
var defaults = {};

// Global variables.
var settings = {};
var wamp;

// Controllers.
var ctrlLoad = {};
var ctrlMenu ={};
var ctrlModal = {};
var ctrlWamp = {};

// Main
$(document).ready(function() {
    // Begin loading.
    ctrlLoad.loadTabs();
    ctrlMenu.loadSettings();

    $('.ui.dropdown').dropdown({
        action: 'hide'
    });

    $('.menu .item').tab();

    rivets.bind($('#menu'), {data: settings.style, run: ctrlMenu});

    // End loading.
    $('.dimmer.active').removeClass('active');
});

// Prevent accidental closing.
$(window).bind('beforeunload', function() {
    ctrlMenu.saveSettings();
    return "Unloading will cause loss of current mission state. Are you sure you want to do this?";
});