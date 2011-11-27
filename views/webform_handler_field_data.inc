<?php
// $Id$

/**
* @file
* Views handler to display the submitted data of a Webform component.
*/

/**
 * Field handler to present basic/text content from the webform_submitted_table.
 */
class webform_handler_field_data extends views_handler_field {

  // Boolean to tell us an others that we are cloned...
  public $cloned = FALSE;

  // Save the component that this field represents.
  public $component = NULL;

  function option_definition() {
    $options = parent::option_definition();
    $options['nid'] = array('default' => '');
    return $options;
  }

  /**
   * Default options form that provides the label widget that all fields
   * should have.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['nid'] = array(
      '#type' => 'textfield',
      '#title' => t('Webform NID'),
      '#default_value' => $this->options['nid'],
      '#description' => t('Enter the Webform NID that will be used for pulling in submitted Webform data.'),
    );
  }

  /**
   * Taken from http://drupal.org/node/66183....
   *
   * @param type $arr1
   * @param type $key
   * @param type $arr2
   * @param type $before
   * @return type
   */
  function array_insert($arr1, $key, $arr2, $before = FALSE) {
    $index = array_search($key, array_keys($arr1));
    if($index === FALSE){
      $index = count($arr1); // insert @ end of array if $key not found
    }
    else {
      if(!$before){
        $index++;
      }
    }
    $end = array_splice($arr1, $index);
    return array_merge($arr1, $arr2, $end);
  }

  // Clone this field as another field...
  function clone_field() {

    // Clone this handler...
    $class_name = get_class($this);
    $handler = new $class_name();

    // Init this handler.
    $handler->init($this->view, $this->options);

    // Set this as a clone.
    $handler->cloned = TRUE;

    // Return the new handler.
    return $handler;
  }

  // Get the webform object.
  function get_webform() {
    static $webform = null;
    if (!$webform) {

      // Load the node.
      $webform = node_load($this->options['nid']);
    }

    // Return the webform object.
    return $webform;
  }

  // Query function for this field.
  function pre_query() {

    // Only continue if this is the base field...
    if (!$this->cloned && empty($this->alias)) {

      $this->ensure_my_table();

      // This is typically created with a relationship handler... however,
      // we need to dynamically build relationships for each component within the
      // webform.  Because of this, we will need to create the relationship here.
      $join = new views_join();
      $join->definition = array(
        'base_field' => 'nid',
        'base' => 'webform_submitted_data',
        'handler' => 'views_handler_relationship',
        'field' => 'sid',
        'left_field' => 'sid',
        'left_table' => 'webform_submissions',
        'relationship field' => 'nid',
        'relationship table' => 'webform_submissions',
        'skip base' => array('node', 'node_revisions'),
        'table' => 'webform_submitted_data'
      );
      $join->construct();
      $join->adjusted = TRUE;

      // Get the webform... make a function so that other classes can override this
      // and figure out better ways to get the webform.
      $webform = $this->get_webform();

      // Iterate through all of our components...
      $index = 0;
      $last_handler = $this->field;

      // Include the webform components.
      module_load_include('inc', 'webform', 'includes/webform.components');

      // Check to make sure that this webform has components.
      if ($webform && $webform->webform['components']) {

        // Add the submission ID to the fields.
        $this->query->add_field($this->query->table_queue['webform_submissions']['alias'], 'sid', 'webform_submissions_sid');

        // Add the webform node ID to the join of the submissions table.
        $this->view->query->table_queue['webform_submissions']['join']->extra = array(
          array(
            'field' => 'nid',
            'operator' => '=',
            'value' => $webform->nid,
            'numeric' => TRUE,
          )
        );

        // Iterate through all the webform components.
        foreach ($webform->webform['components'] as $cid => $component) {

          // Only include component fields that are input types.
          if (webform_component_is_input($component)) {

            // Get the new field handler for this field.
            $handler = ($index > 0) ? $this->clone_field() : $this;

            // Make sure we clone so that we get a non-reference of the join object.
            $newjoin = clone $join;

            // Add a join.
            $newjoin->extra = array(
              array(
                'field' => 'cid',
                'operator' => '=',
                'value' => $cid,
                'numeric' => TRUE,
              )
            );

            // Create an alias for this relationship.
            $alias = 'webform_submitted_data_' . $cid;

            // Add our table alias.
            $relationship->alias = $handler->query->add_relationship($alias, $newjoin, 'webform_submitted_data');

            // Save the component.
            $handler->component = $component;

            // Now add the field we actually want to retreive as data.
            $handler->alias = $handler->query->add_field($relationship->alias, 'data', ('webform_submitted_data_node_' . $webform->nid . '_cid_' . $cid));

            // Change the label to match the component name.
            $handler->options['label'] = $component['name'];

            if ($index > 0) {
              // Insert the new handler and retain field orders...
              $handler->options['id'] = $handler->alias;
              $handler->view->display_handler->handlers['field'] = $handler->array_insert($handler->view->display_handler->handlers['field'], $last_handler, array($handler->alias => $handler));
              $handler->view->field = $handler->array_insert($handler->view->field, $last_handler, array($handler->alias => $handler));
            }

            // Increment our index.
            $last_handler = ($index == 0) ? $last_handler : $handler->alias;
            $index++;
          }
        }
      }
      else {

        // If there are no components, then exclude this field from view.
        $this->options['exclude'] = 1;
        unset($this->view->field['submitted']);
        unset($this->query->table_queue['webform_submissions']);
      }

      return TRUE;
    }

    return FALSE;
  }

  function query() {}

  /**
   * Render a single row...
   *
   * @param type $values
   * @return type
   */
  function render($values) {

    // Check to make sure we have a value.
    if (isset($this->alias) && isset($values->{$this->alias})) {

      // Check to see if there are more values...
      if ($this->component['extra']['multiple']) {

        // Get all the data for this specific multi-value data submission...
        $data = '';
        $result = db_query("SELECT data FROM {webform_submitted_data} WHERE nid=%d AND sid=%d AND cid=%d", $this->component['nid'], $values->webform_submissions_sid, $this->component['cid']);
        while ($value = db_fetch_object($result)) {
          $data .= webform_component_invoke($this->component['type'], 'table', $this->component, array($value->data)) . '<br/>';
        }
        return rtrim($data, '<br/>');
      }
      else {

        // Get the value.
        $data = $values->{$this->alias};
        return webform_component_invoke($this->component['type'], 'table', $this->component, array($data));
      }
    }
    else {
      return '';
    }
  }
}