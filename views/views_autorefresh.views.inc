<?php
/**
 * @file
 * Views definitions of module Views Auto-Refresh.
 */

/**
 * Implements hook_views_data_alter().
 */
function views_autorefresh_views_data_alter(&$data) {
  // @todo is this the right hook?
  $data['views']['requesttime'] = array(
    'title' => t('Request time'),
    'help' => t('Shows the time this view has been fetched from the server.'),
    'area' => array(
      'handler' => 'views_autorefresh_handler_area_requesttime',
    ),
  );

  // @TODO determine if this should go into "if" statement below
  // Add our autorefresh area to Global.
  $data['views']['autorefresh'] = array(
    'title' => t('Autorefresh'),
    'help' => t('Enable autorefresh for this view. NOTE: This will automatically turn AJAX support ON.'),
    'area' => array(
      'handler' => 'views_autorefresh_handler_area_autorefresh',
    ),
  );

  if ($data) {
    foreach ($data as $table_name => $table_info) {
      if ($table_info) {
        foreach ($table_info as $field_name => $field_info) {
          // Add an operator-based date handler to all date arguments.
          if (!empty($field_info['field']['handler']) && $field_info['field']['handler'] == 'views_handler_field_date') {
            $date = $field_info;
            $date['title'] = $field_info['title'] . ' (with operator)';
            $date['argument']['handler'] = 'views_autorefresh_handler_argument_date';
            $date['argument']['field'] = !empty($date['field']['field']) ? $date['field']['field'] :
              (!empty($date['real field']) ? $date['real field'] : $field_name);

            unset($date['filter'], $date['field'], $date['relationship'], $date['sort']);

            $data[$table_name][$field_name . '_operator'] = $date;
          }
        }
      }

      // Add an operator-based argument on the primary field of each base table.
      $field_name = '';
      if (isset($table_info['table']) && !empty($table_info['table']['base'])) {
        $field_name = $table_info['table']['base']['field'];
      }

      if (!empty($field_name) && is_string($field_name) && !empty($table_info[$field_name])) {
        $field_info = $table_info[$field_name];

        if (!empty($field_info['argument']['handler'])) {
          $base = $field_info;
          $base['title'] = $field_info['title'] . ' (with operator)';
          $base['argument']['handler'] = 'views_autorefresh_handler_argument_base';
          $base['argument']['field'] = $field_name;

          unset($base['filter'], $base['field'], $base['relationship'], $base['sort']);

          $data[$table_name][$field_name . '_operator'] = $base;
        }
      }
    }
  }
}
