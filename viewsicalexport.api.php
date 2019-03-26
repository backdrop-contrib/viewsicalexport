<?php
/**
 * @file
 * Documentation for the hooks provided by Views iCal Export.
 */

/**
 * Alter the HTML from an event's text fields before they get exported.
 */
function hook_viewsicalexport_html_alter(&$text_fields, $view, $context) {

}

/**
 * Modify an event's raw data.
 */
function hook_viewsicalexport_raw_event_alter(&$event, $view, $context) {

}

/**
 * Alter a VEVENT array.
 */
function hook_viewsicalexport_vevent_alter(&$vevent, $view, $event_array) {

}

/**
 * Alter the vcalendar array before it's exported as an iCal feed.
 */
function hook_viewsicalexport_vcalendar_alter(&$vcalendar, $view) {

}

/**
 * Alter the final rendered text of a vcalendar before it gets exported.
 */
function hook_viewsicalexport_post_render_alter(&$rendered_calendar, $view) {

}
