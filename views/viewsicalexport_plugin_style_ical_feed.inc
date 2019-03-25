<?php
/**
 * @file
 * Views style plugin for the Date iCal module.
 */

/**
 * Defines a Views style plugin that renders iCal feeds.
 */
class viewsicalexport_plugin_style_ical_feed extends views_plugin_style {

  /**
   * Internal helper function.
   */
  protected function _get_option($option_name) {
    return isset($this->options[$option_name]) ? $this->options[$option_name] : '';
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
    if (!$this->_get_option('disable_webcal')) {
      $url = str_replace(array('http://', 'https://'), 'webcal://', $url);
    }

    // Render the feed icon and header tag (except during a View Preview).
    if (empty($this->view->live_preview)) {
      $tooltip = t('Add to My Calendar');
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
      '#default_value' => $this->_get_option('cal_name'),
      '#description' => t('This will appear as the title of the iCal feed. If left blank, the View Title will be used.
        If that is also blank, the site name will be inserted as the iCal feed title.'),
    );
    $form['no_calname'] = array(
      '#type' => 'checkbox',
      '#title' => t('Exclude Calendar Name'),
      '#default_value' => $this->_get_option('no_calname'),
      '#description' => t("Excluding the X-WR-CALNAME value from the iCal Feed causes
        some calendar clients to add the events in the feed to an existing calendar, rather
        than creating a whole new calendar for them."),
    );
    $form['disable_webcal'] = array(
      '#type' => 'checkbox',
      '#title' => t('Disable webcal://'),
      '#default_value' => $this->_get_option('disable_webcal'),
      '#description' => t("By default, the feed URL will use the webcal:// scheme, which allows calendar
        clients to easily subscribe to the feed. If you want your users to instead download this iCal
        feed as a file, activate this option."),
    );
    $form['exclude_dtstamp'] = array(
      '#type' => 'checkbox',
      '#title' => t('Exclude DTSTAMP'),
      '#default_value' => $this->_get_option('exclude_dtstamp'),
      '#description' => t("By default, the feed will set each event's DTSTAMP property to the time at which the feed got downloaded.
        Some feed readers will (incorrectly) look at the DTSTAMP value when they compare different downloads of the same feed, and
        conclcude that the event has been updated (even though it hasn't actually changed). Enable this option to exclude the DTSTAMP
        field from your feeds, so that these buggy feed readers won't mark every event as updated every time they check."),
    );
    $form['unescape_punctuation'] = array(
      '#type' => 'checkbox',
      '#title' => t('Unescape Commas and Semicolons'),
      '#default_value' => $this->_get_option('unescape_punctuation'),
      '#description' => t('In order to comply with the iCal spec, Date iCal will "escape" commas and semicolons (prepend them with backslashes).
        However, many calendar clients are bugged to not unescape these characters, leaving the backslashes littered throughout your events.
        Enable this option to have Date iCal unescape these characters before it exports the iCal feed.'),
    );
  }

  /**
   * Render the event arrays returned by the row plugin into a VCALENDAR.
   */
  public function render() {
    if (empty($this->row_plugin) || !in_array($this->row_plugin->plugin_name, array('viewsicalexport', 'viewsicalexport_fields'))) {
      debug('viewsicalexport_plugin_style_ical_feed: This style plugin supports only the "iCal Fields" row plugins.', NULL, TRUE);
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

    // Try to load the iCalcreator library.
    $library = libraries_load('iCalcreator');
    if (!$library['loaded']) {
      // The iCalcreator library isn't available, so we can't output anything.
      $output = t('Please install the iCalcreator library to enable iCal output.');
    }
    else {
      // Create a vcalendar object using the iCalcreator library.
      $config = array('unique_id' => 'Date iCal');
      $vcalendar = new vcalendar($config);
      $vcalendar->setMethod('PUBLISH');

      // Only include the X-WR-CALNAME property if the user didn't enable
      // the "Exclude Calendar Name" option.
      if (!$this->_get_option('no_calname')) {
        $cal_name = $this->_get_option('cal_name');
        if (empty($cal_name)) {
          $cal_name = $this->view->get_title();
          if (empty($cal_name)) {
            $cal_name = config_get('system.core', 'site_name');
          }
        }
        if (!empty($cal_name)) {
          $vcalendar->setProperty('X-WR-CALNAME', $cal_name, array('VALUE' => 'TEXT'));
        }
      }

      // Now add the VEVENTs.
      $timezones = array();
      foreach ($events as $event) {
        if (empty($event)) {
          // The row plugin returned NULL for this row, which can happen due to
          // either various error conditions, or because an RRULE is involved.
          // When this happens, just skip it.
          continue;
        }

        $vevent = $vcalendar->newComponent('vevent');
        $vevent->setUid($event['uid']);
        $vevent->setSummary($event['summary']);

        // Get the start date as an array.
        $start = $event['start']->toArray();
        $start_timezone = $event['start']->getTimezone()->getName();
        $timezones[$start_timezone] = $start_timezone;

        if ($event['all_day']) {
          // All Day events need to be DATEs, rather than DATE-TIMEs.
          $vevent->setDtstart($start['year'], $start['month'], $start['day'],
            FALSE, FALSE, FALSE, FALSE, array('VALUE' => 'DATE'));
        }
        else {
          $vevent->setDtstart(
            $start['year'],
            $start['month'],
            $start['day'],
            $start['hour'],
            $start['minute'],
            $start['second'],
            $start_timezone
          );
        }

        // Add the Timezone info to the start date, for use later.
        $start['tz'] = $event['start']->getTimezone();

        // Only add the end date if there is one.
        if (!empty($event['end'])) {
          $end = $event['end']->toArray();
          $end_timezone = $event['end']->getTimezone()->getName();
          $timezones[$end_timezone] = $end_timezone;

          if ($event['all_day']) {
            $vevent->setDtend($end['year'], $end['month'], $end['day'],
              FALSE, FALSE, FALSE, FALSE, array('VALUE' => 'DATE'));
          }
          else {
            $vevent->setDtend(
              $end['year'],
              $end['month'],
              $end['day'],
              $end['hour'],
              $end['minute'],
              $end['second'],
              $end_timezone
            );
          }
          $end['tz'] = $event['end']->getTimezone();
        }

        if (!empty($event['url'])) {
          $vevent->setUrl($event['url'], array('type' => 'URI'));
        }
        if (!empty($event['location'])) {
          $vevent->setLocation($event['location']);
        }
        if (!empty($event['description'])) {
          $vevent->setDescription($event['description']);
        }
        if (!empty($event['last-modified'])) {
          $lm = $event['last-modified']->toArray();
          $vevent->setLastModified(
            $lm['year'],
            $lm['month'],
            $lm['day'],
            $lm['hour'],
            $lm['minute'],
            $lm['second'],
            $lm['timezone']
          );
        }
        if (!empty($event['created'])) {
          $created = $event['created']->toArray();
          $vevent->setCreated(
            $created['year'],
            $created['month'],
            $created['day'],
            $created['hour'],
            $created['minute'],
            $created['second'],
            $created['timezone']
          );
        }

        // Allow other modules to alter the vevent before it's exported.
        backdrop_alter('viewsicalexport_export_vevent', $vevent, $this->view, $event);
      }

      // Now add to the calendar all the timezones used by the events.
      foreach ($timezones as $timezone) {
        if (strtoupper($timezone) != 'UTC') {
          iCalUtilityFunctions::createTimezone($vcalendar, $timezone);
        }
      }

      // Allow other modules to alter the vcalendar before it's exported.
      backdrop_alter('viewsicalexport_export_vcalendar', $vcalendar, $this->view);

      $output = $vcalendar->createCalendar();
      // iCalcreator escapes all commas and semicolons in string values, as the
      // spec demands. However, some calendar clients are buggy and fail to
      // unescape these characters. Users may choose to unescape them here to
      // sidestep those clients' bugs.
      // NOTE: This results in a non-compliant iCal feed, but it seems like a
      // LOT of major clients are bugged this way.
      if ($this->_get_option('unescape_punctuation')) {
        $output = str_replace('\,', ',', $output);
        $output = str_replace('\;', ';', $output);
      }

      // In order to respect the Exclude DTSTAMP option, we unfortunately have
      // to parse out the DTSTAMP properties after they get rendered. Simply
      // using deleteProperty('DTSTAMP') doesn't work, because iCalcreator
      // considers the DTSTAMP to be essential, and will re-create it when
      // createCalendar() is called.
      if ($this->_get_option('exclude_dtstamp')) {
        $filtered_lines = array();
        foreach (explode("\r\n", $output) as $line) {
          if (strpos($line, 'DTSTAMP') === 0) {
            continue;
          }
          $filtered_lines[] = $line;
        }
        $output = implode("\r\n", $filtered_lines);
      }
    }

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
    backdrop_alter('viewsicalexport_export_post_render', $output, $this->view);

    return $output;
  }
}
