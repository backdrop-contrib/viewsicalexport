<?php

/**
 * @file
 * Views style plugin for the Views iCal Export module.
 */

/**
 * Defines a Views style plugin that renders iCal feeds.
 */
class ViewsicalexportPluginStyleIcalFeed extends views_plugin_style {

  /**
   * Internal helper function.
   */
  protected function getOption($option_name) {
    return isset($this->options[$option_name]) ? $this->options[$option_name] : '';
  }

  /**
   * Helper function that creates date string from object.
   *
   * @param object $date
   *   BackdropDateTime object.
   * @param bool $date_only
   *   Whether cardinality should be full or date only.
   */
  protected function dateObjectToString($date, $date_only = FALSE) {
    $stamp = $date->format('U');
    if ($date_only) {
      $tz = $date->getTimezone();
      // Use original timezone for date_only, as UTC would lead to
      // unexpected results then.
      $formatted = format_date($stamp, 'custom', 'Ymd', $tz->getName());
    }
    else {
      $formatted = format_date($stamp, 'custom', 'Ymd\THis\Z', 'UTC');
    }
    return $formatted;
  }

  /**
   * Sets up the iCal feed icon on pages.
   */
  public function attach_to($display_id, $path, $title) {
    $url_options = array();
    $input = $this->view->get_exposed_input();
    if ($input) {
      $url_options['query'] = $input;
    }
    $url_options['absolute'] = TRUE;

    $url = url($this->view->get_url(NULL, $path), $url_options);
    // If the user didn't disable the option, change the scheme to webcal://
    // so calendar clients can automatically subscribe via the iCal link.
    if (!$this->getOption('disable_webcal')) {
      $url = str_replace(array('http://', 'https://'), 'webcal://', $url);
    }

    // Render the feed icon and header tag (except during a View Preview).
    if (empty($this->view->live_preview)) {
      $tooltip = t('Subscribe to this calendar');
      if (!isset($this->view->feed_icon)) {
        // In PHP 5.5, you're not allowed to concatinate onto an unset
        // property. But we need to do a concat, because there may be
        // other attachments.
        $this->view->feed_icon = '';
      }
      $variables = array(
        'url' => check_url($url),
        'tooltip' => $tooltip,
        'view' => $this->view,
      );
      $this->view->feed_icon .= theme('viewsicalexport_icon', $variables);
      backdrop_add_html_head_link(array(
        'rel' => 'alternate',
        'type' => 'text/calendar',
        'title' => $tooltip,
        'href' => $url,
      ));
    }
  }

  /**
   * Set up the options for the style plugin.
   */
  public function option_definition() {
    $false_bool = array('default' => FALSE, 'bool' => TRUE);

    $options = parent::option_definition();
    $options['cal_name'] = array('default' => array());
    $options['no_calname'] = $false_bool;
    $options['disable_webcal'] = $false_bool;
    $options['exclude_dtstamp'] = $false_bool;
    $options['unescape_punctuation'] = $false_bool;
    return $options;
  }

  /**
   * Build the form for setting the style plugin's options.
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    // Allow users to override the default Calendar name (X-WR-CALNAME).
    $form['cal_name'] = array(
      '#type' => 'textfield',
      '#title' => t('iCal Calendar Name'),
      '#default_value' => $this->getOption('cal_name'),
      '#description' => t('This will appear as the title of the iCal feed. If left blank, the View Title will be used.
        If that is also blank, the site name will be inserted as the iCal feed title.'),
    );
    $form['no_calname'] = array(
      '#type' => 'checkbox',
      '#title' => t('Exclude Calendar Name'),
      '#default_value' => $this->getOption('no_calname'),
      '#description' => t("Excluding the X-WR-CALNAME value from the iCal Feed causes
        some calendar clients to add the events in the feed to an existing calendar, rather
        than creating a whole new calendar for them."),
    );
    $form['disable_webcal'] = array(
      '#type' => 'checkbox',
      '#title' => t('Disable webcal://'),
      '#default_value' => $this->getOption('disable_webcal'),
      '#description' => t("By default, the feed URL will use the webcal:// scheme, which allows calendar
        clients to easily subscribe to the feed. If you want your users to instead download this iCal
        feed as a file, activate this option."),
    );
    $form['exclude_dtstamp'] = array(
      '#type' => 'checkbox',
      '#title' => t('Exclude DTSTAMP'),
      '#default_value' => $this->getOption('exclude_dtstamp'),
      '#description' => t("By default, the feed will set each event's DTSTAMP property to the time at which the feed got downloaded.
        Some feed readers will (incorrectly) look at the DTSTAMP value when they compare different downloads of the same feed, and
        conclcude that the event has been updated (even though it hasn't actually changed). Enable this option to exclude the DTSTAMP
        field from your feeds, so that these buggy feed readers won't mark every event as updated every time they check."),
    );
    $form['unescape_punctuation'] = array(
      '#type' => 'checkbox',
      '#title' => t('Unescape Commas and Semicolons'),
      '#default_value' => $this->getOption('unescape_punctuation'),
      '#description' => t('In order to comply with the iCal spec, Views iCal Export will "escape" commas and semicolons (prepend them with backslashes).
        However, many calendar clients are bugged to not unescape these characters, leaving the backslashes littered throughout your events.
        Enable this option to skip escaping.'),
    );
  }

  /**
   * Render the event arrays returned by the row plugin into a VCALENDAR.
   */
  public function render() {
    if (empty($this->row_plugin) || $this->row_plugin->plugin_name !== 'viewsicalexport_fields') {
      debug('ViewsicalexportPluginStyleIcalFeed: This style plugin supports only the "iCal Fields" row plugin.', NULL, TRUE);
      return t('To enable iCal output, the view Format must be configured to Show: iCal Fields.');
    }
    if ($this->row_plugin->plugin_name == 'viewsicalexport_fields' && empty($this->row_plugin->options['date_field'])) {
      // Because the Date field is required by the form, this error state will
      // rarely occur. But I ran across it during testing, and the error that
      // resulted was totally non-sensical, so I'm adding this just in case.
      return t("When using the iCal Fields row plugin, the Date field is required. Please set it up using the Settings link under 'Format -> Show: iCal Fields'.");
    }
    $events = array();
    foreach ($this->view->result as $row_index => $row) {
      $this->view->row_index = $row_index;
      $row->index = $row_index;
      try {
        $events[] = $this->row_plugin->render($row);
      }
      catch (Exception $e) {
        debug($e->getMessage(), NULL, TRUE);
        return $e->getMessage();
      }
    }
    unset($this->view->row_index);

    $vcalendar = array();
    $dtstamp = format_date(REQUEST_TIME, 'custom', 'Ymd\THis\Z', 'UTC');
    $vcalendar[] = 'BEGIN:VCALENDAR';
    $vcalendar[] = 'VERSION:2.0';
    $vcalendar[] = 'PRODID:-//Backdrop Views Ical Export';
    $vcalendar[] = 'METHOD:PUBLISH';

    // Only include the X-WR-CALNAME property if the user didn't enable
    // the "Exclude Calendar Name" option.
    if (!$this->getOption('no_calname')) {
      $cal_name = $this->getOption('cal_name');
      if (empty($cal_name)) {
        $cal_name = $this->view->get_title();
        if (empty($cal_name)) {
          $cal_name = config_get('system.core', 'site_name');
        }
      }
      if (!empty($cal_name)) {
        $vcalendar[] = 'X-WR-CALNAME;VALUE=TEXT:' . $cal_name;
      }
    }

    // Now add the VEVENTs.
    foreach ($events as $event) {
      $vevent = array();
      if (empty($event)) {
        // The row plugin returned NULL for this row.
        // When this happens, just skip it.
        continue;
      }

      $vevent[] = 'BEGIN:VEVENT';
      $vevent[] = 'UID:' . $event['uid'];
      if (!$this->getOption('exclude_dtstamp')) {
        $vevent[] = 'DTSTAMP:' . $dtstamp;
      }
      $vevent[] = 'SUMMARY:' . $event['summary'];

      if ($event['all_day']) {
        // All Day events need to be DATEs, rather than DATE-TIMEs.
        $vevent[] = 'DTSTART;VALUE=DATE:' . $this->dateObjectToString($event['start'], TRUE);
      }
      else {
        $vevent[] = 'DTSTART:' . $this->dateObjectToString($event['start']);
      }

      // Only add the end date if there is one.
      if (!empty($event['end'])) {
        if ($event['all_day']) {
          $vevent[] = 'DTEND;VALUE=DATE:' . $this->dateObjectToString($event['end'], TRUE);
        }
        else {
          $vevent[] = 'DTEND:' . $this->dateObjectToString($event['end']);
        }
      }
      if (!empty($event['location'])) {
        $vevent[] = 'LOCATION:' . $event['location'];
      }
      if (!empty($event['description'])) {
        $vevent[] = 'DESCRIPTION:' . $event['description'];
      }
      if (!empty($event['last-modified'])) {
        $vevent[] = 'LAST-MODIFIED:' . $this->dateObjectToString($event['last-modified']);
      }
      if (!empty($event['created'])) {
        $vevent[] = 'CREATED:' . $this->dateObjectToString($event['created']);
      }
      if (!empty($event['url'])) {
        $vevent[] = wordwrap('ATTACH:' . $event['url'], 75, "\r\n ", TRUE);
      }
      $vevent[] = 'END:VEVENT';

      // Allow other modules to alter the vevent before it's exported.
      backdrop_alter('viewsicalexport_vevent', $vevent, $this->view);
      $vcalendar[] = implode("\r\n", $vevent);
    }

    $vcalendar[] = 'END:VCALENDAR';

    // Allow other modules to alter the vcalendar before it's exported.
    backdrop_alter('viewsicalexport_vcalendar', $vcalendar, $this->view);
    $output = implode("\r\n", $vcalendar);

    // These steps shouldn't be run during Preview on the View page.
    if (empty($this->view->live_preview)) {
      // Prevent devel module from appending queries to ical export.
      $GLOBALS['devel_shutdown'] = FALSE;

      backdrop_add_http_header('Content-Type', 'text/calendar; charset=UTF-8');
      backdrop_add_http_header('Cache-Control', 'no-cache, must-revalidate');
      backdrop_add_http_header('Expires', 'Sat, 26 Jul 1997 05:00:00 GMT');

      // For sites with Clean URLs disabled, the Display's "path" value ends
      // up only in the query args, meaning the filename won't be set properly
      // when users download the feed. So we need to manually instruct browsers
      // to download a .ics file.
      if (!config_get('system.core', 'clean_url')) {
        $path_array = explode('/', $this->display->display_options['path']);
        $filename = end($path_array);
        backdrop_add_http_header('Content-Disposition', "attachment; filename=\"$filename\"");
      }
    }

    // Allow other modules to alter the rendered calendar.
    backdrop_alter('viewsicalexport_post_render', $output, $this->view);

    return $output;
  }

}
