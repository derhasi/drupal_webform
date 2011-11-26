<?php
/**
 * @file
 * Views' relationship handlers.
 */

class webform_handler_relationship_webform_data extends views_handler_relationship  {

  function option_definition() {
    $options = parent::option_definition();
    $options['components'] = array('default' => array());
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $options = $this->_get_used_webform_components();
    $form['components'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Components'),
      '#options' => $options,
      '#default_value' => empty($this->options['components']) ? $options : $this->options['components'],
      '#description' => t('Choose which components you wish to relate. On no selection, all components are joined.'),
    );
    parent::options_form($form, $form_state);
  }

  /**
   * Retrieve a list of defined components for the given component type.
   *
   * @TODO: add wrapper function in webform.module
   * @TODO: cids are not really exportable
   */
  function _get_used_webform_components() {
    // Generate a lookup table of results.
    $component_type = $this->definition['webform component type'];
    $query = db_select('webform_component', 'wc')
            ->fields('wc', array('nid', 'cid', 'name', 'form_key'))
            ->condition('type', $component_type );

    $result = $query->execute();

    $components = array();
    foreach ($result as $data) {
      // Component primary key is (nid, cid).
      $components["{$data->nid}:{$data->cid}"] = check_plain("{$data->name} ({$data->form_key})");
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
    $components = (empty($this->options['components'])) ? $this->_get_used_webform_components() : $this->options['components'];

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
