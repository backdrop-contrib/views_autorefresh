<?php
/**
 * @file
 * Definition of views_autorefresh_handler_argument_base.
 */

class views_autorefresh_handler_argument_base extends views_handler_argument {
  /**
   * Construct the instance.
   */
  public function construct() {
    parent::construct();

    $this->operator = '>';
  }

  /**
   * Build the query based upon the formula.
   */
  public function query($group_by = FALSE) {
    $this->ensure_my_table();

    $this->query->add_where_expression(0, "{$this->table_alias}.{$this->real_field} {$this->operator} :base", array(':base' => $this->argument));
  }
}
