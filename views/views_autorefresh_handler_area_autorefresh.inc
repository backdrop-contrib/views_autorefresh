<?php
/**
 * @file
 * Base class for area handlers.
 *
 * @ingroup views_area_handlers
 */

class views_autorefresh_handler_area_autorefresh extends views_handler_area {
  /**
   * Overrides views_handler_area::init().
   *
   * Reset override done in views_handler_area::init(). This area must be
   * rendered even if view has no results.
   */
  public function init(&$view, &$options) {
    parent::init($view, $options);
  }

  /**
   * Extend option definitions.
   */
  public function option_definition() {
    $options = parent::option_definition();
    $options['interval'] = array('default' => '');
    $options['nodejs'] = array('default' => FALSE, 'bool' => TRUE);
    $options['incremental'] = array('default' => FALSE, 'bool' => TRUE);
    $options['display'] = array('default' => '');
    $options['incremental_advanced'] = array(
      'contains' => array(
        'sourceSelector' => array('default' => '.view-content'),
        'targetSelector' => array('default' => '.view-content'),
        'emptySelector' => array('default' => '.view-empty'),
        'afterSelector' => array('default' => '.view-header'),
        'targetStructure' => array('default' => '<div class="view-content"></div>'),
        'firstClass' => array('default' => 'views-row-first'),
        'lastClass' => array('default' => 'views-row-last'),
        'oddClass' => array('default' => 'views-row-odd'),
        'evenClass' => array('default' => 'views-row-even'),
        'rowClassPrefix' => array('default' => 'views-row-'),
      ),
    );
    $options['ping'] = array('default' => FALSE, 'bool' => TRUE);
    $options['ping_base_path'] = array('default' => '');
    $options['ping_arguments'] = array('default' => '');
    $options['trigger_onload'] = array('default' => FALSE, 'bool' => TRUE);

    return $options;
  }

  /**
   * Custom form items.
   */
  public function options_form(&$form, &$form_state) {
    if (module_exists('nodejs')) {
      $form['nodejs'] = array(
        '#type' => 'checkbox',
        '#title' => t('Use Node.js to refresh the view instead of interval pings'),
        '#default_value' => $this->options['nodejs'],
      );
    }
    else {
      $form['nodejs'] = array(
        '#type' => 'value',
        '#value' => FALSE,
      );
    }

    $form['interval'] = array(
      '#type' => 'textfield',
      '#title' => t('Interval to check for new items'),
      '#default_value' => $this->options['interval'],
      '#field_suffix' => 'milliseconds',
      '#required' => !module_exists('nodejs'),
      '#states' => array(
        'invisible' => array(
          ':input[name="options[nodejs]"]' => array('checked' => TRUE),
        ),
        'required' => array(
          ':input[name="options[nodejs]"]' => array('checked' => FALSE),
        ),
      ),
    );
    $form['incremental'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use a secondary view display to incrementally insert new items only'),
      '#default_value' => $this->options['incremental'],
    );

    $options = array();

    if ($this->view->display) {
      foreach ($this->view->display as $display) {
        if ($display->display_plugin == 'page') {
          // @TODO: check secondary display arguments.
          $options[$display->id] = $display->display_title;
        }
      }
    }

    $incremental_advanced = $this->options['incremental_advanced'];

    $form['display'] = array(
      '#type' => 'select',
      '#title' => t('Secondary display'),
      '#default_value' => $this->options['display'],
      '#description' => t('Only displays of type page are eligible. Additionally the display must have a timestamp type Contextual filter <em>(with operator)</em>. For example <em>Content: Post date (with operator)</em>.'),
      '#options' => $options,
      '#states' => array(
        'invisible' => array(
          ':input[name="options[incremental]"]' => array('checked' => FALSE),
        ),
      ),
    );
    $form['incremental_advanced'] = array(
      '#type' => 'fieldset',
      '#title' => t('Advanced'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#description' => t('Advanced settings for incremental display. Unless your view is using an overridden template, the defaults below should be fine.'),
      '#states' => array(
        'invisible' => array(
          ':input[name="options[incremental]"]' => array('checked' => FALSE),
        ),
      ),
    );
    $form['incremental_advanced']['sourceSelector'] = array(
      '#type' => 'textfield',
      '#title' => t('Source selector'),
      '#default_value' => $incremental_advanced['sourceSelector'],
      '#description' => t('A jQuery selector expression representing the main view container of your secondary display.'),
    );
    $form['incremental_advanced']['targetSelector'] = array(
      '#type' => 'textfield',
      '#title' => t('Target selector'),
      '#default_value' => $incremental_advanced['targetSelector'],
      '#description' => t('A jQuery selector expression representing the main view container of your primary display.'),
    );
    $form['incremental_advanced']['emptySelector'] = array(
      '#type' => 'textfield',
      '#title' => t('Empty selector'),
      '#default_value' => $incremental_advanced['emptySelector'],
      '#description' => t('A jQuery selector expression representing the main view container in case of empty results.'),
    );
    $form['incremental_advanced']['afterSelector'] = array(
      '#type' => 'textfield',
      '#title' => t('Header selector'),
      '#default_value' => $incremental_advanced['afterSelector'],
      '#description' => t('A jQuery selector expression representing the view header, in case the header is displayed with empty results.'),
    );
    $form['incremental_advanced']['targetStructure'] = array(
      '#type' => 'textfield',
      '#title' => t('Target structure'),
      '#default_value' => $incremental_advanced['targetStructure'],
      '#description' => t('An HTML fragment describing the view skeleton in case of empty results.'),
    );
    $form['incremental_advanced']['firstClass'] = array(
      '#type' => 'textfield',
      '#title' => t('First row class'),
      '#default_value' => $incremental_advanced['firstClass'],
      '#description' => t('A class to be added to the first result row.'),
    );
    $form['incremental_advanced']['lastClass'] = array(
      '#type' => 'textfield',
      '#title' => t('Last row class'),
      '#default_value' => $incremental_advanced['lastClass'],
      '#description' => t('A class to be added to the last result row.'),
    );
    $form['incremental_advanced']['oddClass'] = array(
      '#type' => 'textfield',
      '#title' => t('Odd rows class'),
      '#default_value' => $incremental_advanced['oddClass'],
      '#description' => t('A class to be added to each odd result row.'),
    );
    $form['incremental_advanced']['evenClass'] = array(
      '#type' => 'textfield',
      '#title' => t('Even rows class'),
      '#default_value' => $incremental_advanced['evenClass'],
      '#description' => t('A class to be added to each even result row.'),
    );
    $form['incremental_advanced']['rowClassPrefix'] = array(
      '#type' => 'textfield',
      '#title' => t('Row class prefix'),
      '#default_value' => $incremental_advanced['rowClassPrefix'],
      '#description' => t('The prefix of a class to be added to each result row. The row number will be appended to this prefix.'),
    );
    $form['ping'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use a ping url'),
      '#default_value' => $this->options['ping'],
      '#description' => t('Use a custom script for faster check of new items. See <code>views_autorefresh_ping.default.php</code> in <code>views_autorefresh</code> folder for reference.'),
    );
    $form['ping_base_path'] = array(
      '#type' => 'textfield',
      '#title' => t('Path to the ping script'),
      '#default_value' => $this->options['ping_base_path'],
      '#description' => t('This path is relative to the Backdrop root.'),
      '#states' => array(
        'invisible' => array(
          ':input[name="options[ping]"]' => array('checked' => FALSE),
        ),
      ),
    );
    $form['ping_arguments'] = array(
      '#type' => 'textarea',
      '#title' => t('Ping arguments'),
      '#default_value' => $this->options['ping_arguments'],
      '#description' => t('A PHP script that generates arguments that will be sent on the ping URL as query parameters. Do not surround with <code>&lt;?php&gt;</code> tag.'),
      '#states' => array(
        'invisible' => array(
          ':input[name="options[ping]"]' => array('checked' => FALSE),
        ),
      ),
    );
    $form['trigger_onload'] = array(
      '#type' => 'checkbox',
      '#title' => t('Trigger Refresh on Load'),
      '#default_value' => $this->options['trigger_onload'],
    );
  }

  /**
   * Custom validation.
   */
  public function options_validate(&$form, &$form_state) {
    if (!$form_state['values']['options']['nodejs'] && !is_numeric($form_state['values']['options']['interval'])) {
      form_set_error('interval', t('Invalid interval.'));
    }

    if ($form_state['values']['options']['ping']) {
      $ping_base_path = BACKDROP_ROOT . '/' . $form_state['values']['options']['ping_base_path'];

      if (!file_exists($ping_base_path)) {
        form_set_error('ping_base_path', t('Ping script not found at %path.', array('%path' => $ping_base_path)));
      }

      $args = $this->eval_ping_arguments($form_state['values']['options']['ping_arguments']);

      if (!is_array($args)) {
        form_set_error('ping_arguments', t('Error in ping arguments script: %error', array('%error' => $args)));
      }
    }
  }

  /**
   * Custom submit handling.
   */
  public function options_submit(&$form, &$form_state) {
    $this->view->display_handler->set_option('use_ajax', TRUE);
  }

  /**
   * Render output.
   */
  public function render($empty = FALSE) {
    $args = array();
    $args['view'] = $this->view;
    $args['nodejs'] = !empty($this->options['nodejs']);
    $args['interval'] = $this->options['interval'];

    if ($this->options['ping']) {
      $args['ping'] = array(
        'ping_base_path' => $this->options['ping_base_path'],
        'ping_args' => $this->eval_ping_arguments($this->options['ping_arguments']),
      );
    }

    $args['trigger_onload'] = $this->options['trigger_onload'];

    if ($this->options['incremental']) {
      $display = $this->view->display[$this->options['display']];
      $incremental_advanced = $this->options['incremental_advanced'];

      $args['incremental'] = array(
        'view_base_path' => $display->display_options['path'],
        'view_display_id' => $display->id,
        'view_name' => $this->view->name,
        'sourceSelector' => $incremental_advanced['sourceSelector'],
        'targetSelector' => $incremental_advanced['targetSelector'],
        'emptySelector' => $incremental_advanced['emptySelector'],
        'afterSelector' => $incremental_advanced['afterSelector'],
        'targetStructure' => $incremental_advanced['targetStructure'],
        'firstClass' => $incremental_advanced['firstClass'],
        'lastClass' => $incremental_advanced['lastClass'],
        'oddClass' => $incremental_advanced['oddClass'],
        'evenClass' => $incremental_advanced['evenClass'],
        'rowClassPrefix' => $incremental_advanced['rowClassPrefix'],
      );
    }

    return theme('views_autorefresh', $args);
  }

  /**
   * Custom error handler.
   */
  public function errorHandler($errno, $errstr) {
    return FALSE;
  }

  /**
   * Custom method.
   */
  public function eval_ping_arguments($script) {
    $args = array();

    if (empty($script)) {
      return $args;
    }

    // @TODO determine if necessary
    // Make view visible to script.
    $view = $this->view;

    // Avoid Backdrop's error handler.
    // @see http://www.php.net/manual/en/function.restore-error-handler.php#93261
    set_error_handler(array($this, 'errorHandler'));

    // @todo Remove the evil.
    $return = eval($script);

    if ($return === FALSE) {
      $error = error_get_last();
      $args = $error['message'];
    }
    elseif (is_array($return)) {
      $args = $return;
    }
    else {
      $args = t('expecting an array of arguments, got a !type instead.', array('!type' => gettype($return)));
    }

    restore_error_handler();

    return $args;
  }
}
