<?php
/**
 * @file
 * Views' relationship handlers.
 */

class webform_handler_relationship_webform_data extends views_handler_relationship  {

  protected $component_instances;

  function option_definition() {
    $options = parent::option_definition();
    $options['components'] = array('default' => array());
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $options = $this->get_webform_component_instances();
    $form['components'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Components'),
      '#options' => $options,
      '#default_value' => empty($this->options['components']) ? $options : $this->options['components'],
      '#description' => t('Choose which components you wish to relate. On no selection, all components are joined.'),
    );

    // @TODO: add delta selection.
  }

  /**
   * Retrieve a list of defined components for the given component type.
   *
   * The function builds the $this->component_instances variable, with the full
   * array data for a given instance. It outputs a flat array for usage in
   * #options.
   *
   * @TODO: cids are not really exportable
   *
   * @return array
   *  array of component instances keyed by primary key "nid:cid" and with a
   *  label as value.
   */
  function get_webform_component_instances() {

    if (!isset($this->component_instances)) {
      $component_type = $this->definition['webform component type'];
      $components = webform_component_instances();
      foreach ($components as $key => $component) {
        if ($component['type'] == $component_type) {
          $this->component_instances[$key] = $component;
        }
      }
    }

    $components = array();
    foreach ($result as $data) {
      // Component primary key is (nid, cid).
      $components["{$data->nid}:{$data->cid}"] = check_plain("Webform {$data->nid}: {$data->name} ({$data->form_key})");
    }
    return $components;
  }

  /**
   * Called to implement a relationship in a query.
   *
   * I respects the given component ids, provided via options form.
   */
  function query() {

    // On no selection, all components are joined.
    $components = (empty($this->options['components'])) ? $this->get_webform_component_instances() : $this->options['components'];

    // Add the extra to the definition to filter on component ids.
    $this->definition['extra'] = array(
      array(
        'table' => NULL,
        'field' => "CONCAT(%alias.nid, ':', %alias.cid)",
        'value' => $components,
      ),
    );

    // The rest of building the join is performed by the parent.
    parent::query();
  }

}
