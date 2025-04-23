# Views iCal Export

This module enables you to export iCal feeds (also known as vcalendar) from views, using the
"iCal Feed" and "iCal Fields" views plugins.

A typical use case is a view from event nodes (any content type with date fields). 
Using this module, you can attach an iCal feed to your views' page, so visitors are able
to add your events to their calendar and get them updated automatically.

![Icon screenshot](https://raw.githubusercontent.com/backdrop-contrib/viewsicalexport/1.x-1.x/screenshots/ical-icon-subscribe-calendar.png)

## Installation

Install this module using the official [Backdrop CMS instructions](https://docs.backdropcms.org/documentation/extend-with-modules).

Create or edit a view from a content type with date fields (could be core date,
could be repeating dates).

To attach an iCal feed to that view, click on "+ Add" next to the Displays
buttons. Choose FEED. Switch the format (defaults to RSS):

- Format: should be "iCal Feed"
- Show: should be "iCal Fields"

Set an appropriate path under "FEED SETTINGS".
Attach that feed to your main Display - that will show the icon button below
views results on that page/block.

![Admin UI section](https://raw.githubusercontent.com/backdrop-contrib/viewsicalexport/1.x-1.x/screenshots/screenshot-admin-ui.png)

Note that if some of the date fields might be empty, you should either filter
empty field values out via views filter.
Or, if you're using multiple (different) date field types, and use the "First
populated Date field" setting, you can "Skip blank dates" in the iCal Fields setting.
An iCal event obviously needs a date.

## Dependencies

No external dependencies.

Two modules are required, both ship with Backdrop core and are
typically enabled:

* views
* date

## Current maintainers

* Indigoxela (https://github.com/indigoxela)

## Credits

This module is a complete rewrite of the [Date iCal](https://www.drupal.org/project/date_ical) Drupal module.

Date iCal was initially written by [Karen Stevenson](https://www.drupal.org/u/karens), for a long time
maintained by [coredumperror](https://www.drupal.org/u/coredumperror) and ported to Backdrop
by [John Franklin](https://github.com/jlfranklin).
So the original module, this one has been derived from, is also available for Backdrop.

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.
