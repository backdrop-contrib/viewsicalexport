<?php

/**
 * @file
 * Documentation for the hooks provided by Views iCal Export.
 */

/**
 * Alter the HTML from an event's text fields before they get exported.
 *
 * @param array $text_fields
 *   Unchanged text fields (summary, description, location) as provided by the view.
 * @param object $view
 *   The view object that is being executed to render the iCal feed.
 * @param array $context
 *   Associative array of context, with the following keys:
 *   - row
 *   - row_index
 *   - language
 *   - options
 */
function hook_viewsicalexport_field_html_alter(&$text_fields, $view, $context) {

}

/**
 * Modify an event's raw data.
 *
 * @param array $vevent
 *   The vevent array that is being added to the iCal feed.
 *   Sanitizing and escaping has been done now.
 * @param object $view
 *   The view object that is being executed to render the iCal feed.
 * @param array $context
 *   Associative array of context, with the following keys:
 *   - row
 *   - row_index
 *   - language
 *   - options
 */
function hook_viewsicalexport_raw_event_alter(&$event, $view, $context) {

}

/**
 * Alter a VEVENT array.
 *
 * @param array $vevent
 *   The vevent array that is being added to the iCal feed.
 *   Dates have been converted from objects to strings, each array item
 *   represents a (multi-)line item of the VEVENT.
 * @param object $view
 *   The view object that is being executed to render the iCal feed.
 */
function hook_viewsicalexport_vevent_alter(&$vevent, $view) {

}

/**
 * Alter the vcalendar array before it's exported as an iCal feed.
 *
 * @param array $vcalendar
 *   The complete vcalendar array, including all the vevents.
 * @param object $view
 *   The view object that is being executed to render the iCal feed.
 */
function hook_viewsicalexport_vcalendar_alter(&$vcalendar, $view) {

}

/**
 * Alter the final rendered text of a vcalendar before it gets exported.
 *
 * @param string $rendered_calendar
 *   The fully rendered vcalendar.
 * @param object $view
 *   The view object that is being executed to render the iCal feed.
 */
function hook_viewsicalexport_post_render_alter(&$rendered_calendar, $view) {

}
