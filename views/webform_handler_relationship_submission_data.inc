<?php
/**
 * @file
 * Views' relationship handlers.
 */

class webform_handler_relationship_submission_data extends views_handler_relationship  {

  protected $component_instances;

  function option_definition() {
    $options = parent::option_definition();
    $options['component_type'] = array('default' => NULL);
    $options['components'] = array('default' => array());
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $component_types = $this->get_webform_component_types();

    $form['component_type'] = array(
      '#title' => t('Component type'),
      '#type' => 'select',
      '#description' => t('Select component type for this relationship'),
      '#required' => TRUE,
      '#default_value' => $this->options['component_type'],
      '#options' => $component_types,
    );

    ctools_include('dependent');

    foreach ($component_types as $type => $label) {
      $id = drupal_html_id('options-components-'. $type);

      $options = $this->get_webform_component_instances($type);
      // If there are no isntances, we remove the component from selection.
      if (empty($options)) {
        unset($form['component_type']['#options'][$type]);
        if ($form['component_type']['#default_value'] == $type) {
          $form['component_type']['#default_value'] = NULL;
        }
      }
      else {
        $form['components'][$type] = array(
          '#title' => t('Components: %label', array('%label' => $label)),
          '#prefix' => '<div id="'. $id. '">',
          '#suffix' => '</div>',
          '#type' => 'checkboxes',
          '#options' => $options,
          '#default_value' => $this->options['components'],
          '#dependency' => array('edit-options-component-type' => array($type)),
          '#description' => t('Choose which components you wish to relate. On no selection, all components are joined.'),
        );
      }
    }

    // @TODO: add delta selection.
  }

  function get_webform_component_types() {
    $component_types = webform_components();
    foreach ($component_types as $key => $component) {
      $return[$key] = check_plain($component['label']);
    }
    return $return;
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
  function get_webform_component_instances($component_type = NULL) {

    if (!isset($this->component_instances)) {
      $components = webform_component_instances();
      foreach ($components as $key => $component) {
        $this->component_instances[$key] = $component;
      }
    }

    $components = array();
    foreach ($this->component_instances as $data) {
      if (!isset($component_type) || $data['type'] == $component_type) {
        // Component primary key is (nid, cid).
        $components["{$data['nid']}-{$data['cid']}"] = check_plain("Webform {$data['nid']}: {$data['name']} ({$data['form_key']})");
      }
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
    $components = isset($this->options['components']) ? array_filter($this->options['components']) : array();
    $components = (empty($components)) ? array_keys($this->get_webform_component_instances()) : $components;

    // Add the extra to the definition to filter on component ids.
    $this->definition['extra'] = array(
      array(
        'table' => NULL,
        'field' => "CONCAT(%alias.nid, '-', %alias.cid)",
        'value' => $components,
      ),
    );

    // The rest of building the join is performed by the parent.
    parent::query();
  }

}
