<?php
/**
 * @file
 * Definition of views_autorefresh_handler_area_requesttime.
 */

/**
 * Views area handler to display the request time.
 *
 * @ingroup views_area_handlers
 */
class views_autorefresh_handler_area_requesttime extends views_handler_area {

  /**
   * Additional options.
   */
  public function option_definition() {
    $options = parent::option_definition();

    $options['content'] = array(
      'default' => 'Last time refreshed: @requesttime',
      'translatable' => TRUE,
    );
    $options['date_format'] = array('default' => 'html_time');
    $options['custom_date_format'] = array('default' => '');

    return $options;
  }

  /**
   * Enhance the default form with custom settings.
   */
  public function options_form(&$form, &$form_state) {
    $variables = array(
      'items' => array(
        '@requesttime -- the last time this view has been fetched from the server',
      ),
    );
    $list = theme('item_list', $variables);
    $form['content'] = array(
      '#title' => t('Display'),
      '#type' => 'textarea',
      '#rows' => 3,
      '#default_value' => $this->options['content'],
      '#description' => t('You may use HTML code in this field. The following tokens are supported:') . $list,
    );

    // Configurable date format.
    $date_formats = array();
    $date_types = system_get_date_formats();
    foreach ($date_types as $machine_name => $date_type) {
      $date_formats[$machine_name] = $date_type['label'] . ': ' . format_date(REQUEST_TIME, $machine_name);
    }
    $form['date_format'] = array(
      '#type' => 'select',
      '#title' => t('Date format'),
      '#options' => $date_formats + array(
        'custom' => t('Custom'),
      ),
      '#default_value' => isset($this->options['date_format']) ? $this->options['date_format'] : 'html_time',
    );
    $form['custom_date_format'] = array(
      '#type' => 'textfield',
      '#title' => t('Custom date format'),
      '#description' => t('See the <a href="@url" target="_blank">PHP manual</a> for available options.', array(
        '@url' => 'https://www.php.net/manual/datetime.format.php#refsect1-datetime.format-parameters',
      )),
      '#default_value' => isset($this->options['custom_date_format']) ? $this->options['custom_date_format'] : '',
      '#states' => array(
        'visible' => array(
          ':input[name="options[date_format]"]' => array('value' => 'custom'),
        ),
      ),
    );

    parent::options_form($form, $form_state);
  }

  /**
   * Find out the information to render.
   */
  public function render($empty = FALSE) {
    // No results in this view, hide if not set to display on empty results.
    if ($empty && empty($this->options['empty'])) {
      return;
    }

    // Format date according to settings.
    if ($this->options['date_format'] == 'custom' && !empty($this->options['custom_date_format'])) {
      $requesttime = format_date(REQUEST_TIME, 'custom', check_plain($this->options['custom_date_format']));
    }
    else {
      $requesttime = format_date(REQUEST_TIME, $this->options['date_format']);
    }

    $items = array('requesttime');
    $replacements = array();
    foreach ($items as $item) {
      $replacements["@$item"] = ${$item};
    }
    // Send the output.
    $content = $this->options['content'];
    $output = filter_xss_admin(str_replace(array_keys($replacements), array_values($replacements), $content));
    return $output;
  }

}
