<?php

/**
 * @file
 * Declares our plugins to Views.
 */

/**
 * Implements hook_views_plugins().
 */
function viewsicalexport_views_plugins() {
  $includes_path = backdrop_get_path('module', 'viewsicalexport') . '/views';

  $data = array(
    'module' => 'viewsicalexport',
    'style' => array(
      'viewsicalexport' => array(
        'type' => 'feed',
        'title' => t('iCal Feed'),
        'help' => t('Generates an iCal feed from a View.'),
        'handler' => 'ViewsicalexportPluginStyleIcalFeed',
        'path' => $includes_path,
        'uses fields' => FALSE,
        'uses grouping' => FALSE,
        'uses row plugin' => TRUE,
        'uses options' => TRUE,
        'even empty' => TRUE,
      ),
    ),
    'row' => array(
      'viewsicalexport_fields' => array(
        'type' => 'feed',
        'title' => t('iCal Fields'),
        'help' => t('Builds an iCal VEVENT from inidividual Views fields.'),
        'handler' => 'ViewsicalexportPluginRowIcalFields',
        'path' => $includes_path,
        'uses options' => TRUE,
        'uses fields' => TRUE,
      ),
    ),
  );

  return $data;
}
