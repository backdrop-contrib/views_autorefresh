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

  function option_definition() {
    $options = parent::option_definition();

    $options['content'] = array(
      'default' => 'Last time refreshed: @requesttime',
      'translatable' => TRUE,
    );

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
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
  }


  /**
   * Find out the information to render.
   */
  function render($empty = FALSE) {
    // Must have options and does not work on summaries.
    if (!isset($this->options['content']) || $this->view->plugin_name == 'default_summary') {
      return;
    }
    $output = '';
    $format = $this->options['content'];

    // @todo date format configurable.
    $requesttime = format_date(REQUEST_TIME);

    $items = array('requesttime');
    $replacements = array();
    foreach ($items as $item) {
      $replacements["@$item"] = ${$item};
    }
    // Send the output.
    $output = filter_xss_admin(str_replace(array_keys($replacements), array_values($replacements), $format));
    return $output;
  }
}
