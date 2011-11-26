<?php
/**
 * @file
 * Views' relationship handlers.
 */

class webform_handler_relationship_webform_data extends views_handler_relationship  {
  function init(&$view, &$options) {
    parent::init($view, $options);
    
    // Convert legacy vids option to machine name vocabularies.
    if (!empty($this->options['cids'])) {
      $components = webform_components();
      
      
      foreach ($this->options['cids'] as $cid) {
        if (isset($components[$cid], $components[$cid]->machine_name)) {
          $this->options['components'][$components[$cid]->machine_name] = $components[$cid]->machine_name;
        }
      }
    }
  }

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
      '#required' => TRUE,
      '#default_value' => empty($this->options['components']) ? $options : $this->options['components'],
      '#description' => t('Choose which Components you wish to relate.'),
    );
    parent::options_form($form, $form_state);
  }
  
  function _get_used_webform_components() {
    // Generate a lookup table of results.
      $component_type = $this->definition['webform component type'];
      dsm($component_type);
      $query = db_select('webform_component', 'wc')
              ->fields('wc', array('cid', 'name', 'form_key'))
              ->condition('type', $component_type );
      
      $result = $query->execute();
      
      $components = array();
      foreach ($result as $data) {
        $components[$data->form_key] = check_plain("{$data->name} ({$data->form_key})");
      }
      return $components;
  }
  
  
  

  /**
   * Called to implement a relationship in a query.
   */
  /*
  function query() {
    $this->ensure_my_table();

    $def = $this->definition;
    $def['table'] = 'taxonomy_term_data';

    if (!array_filter($this->options['vocabularies'])) {
      $term_node = $this->query->add_table('taxonomy_index', $this->relationship);
      $def['left_table'] = 'taxonomy_index';
      $def['left_field'] = 'tid';
      $def['field'] = 'tid';
      $def['type'] = empty($this->options['required']) ? 'LEFT' : 'INNER';
    }
    else {
      // If vocabularies are supplied join a subselect instead
      $def['left_table'] = $this->table_alias;
      $def['left_field'] = 'nid';
      $def['field'] = 'nid';
      $def['type'] = empty($this->options['required']) ? 'LEFT' : 'INNER';

      $query = db_select('taxonomy_term_data', 'td');
      $query->addJoin($def['type'], 'taxonomy_vocabulary', 'tv', 'td.vid = tv.vid');
      $query->addJoin($def['type'], 'taxonomy_index', 'tn', 'tn.tid = td.tid');
      $query->condition('tv.machine_name', array_filter($this->options['vocabularies']));
      $query->addTag('term_access');
      $query->fields('td');
      $query->fields('tn', array('nid'));
      $def['table formula'] = $query;
    }

    $join = new views_join();

    $join->definition = $def;
    $join->construct();
    $join->adjusted = TRUE;

    // use a short alias for this:
    $alias = $def['table'] . '_' . $this->table;

    $this->alias = $this->query->add_relationship($alias, $join, 'taxonomy_term_data', $this->relationship);
  }
  
  */
  
}
