<?php

/**
 * @file
 * Provide the chunks field.
 */

/**
 * Implements hook_field_info().
 */
function chunks_field_info() {

  // Provide the chunks field.
  $fields = array(
    'chunks' => array(
      'label' => t('Chunks'),
      'description' => t('Structured and reusable content chunks.'),
      'default_widget' => 'chunks_default',
      'settings' => array(),
      'instance_settings' => array('chunk_types' => array(), 'chunk_instances' => array()),
      'default_formatter' => 'chunks_default',
    ),
  );

  // Populate chunks field instance_settings.
  $chunk_types = chunk_types_load();
  foreach ($chunk_types as $chunk_type) {
    $fields['chunks']['instance_settings']['chunk_types'][] = $chunk_type->name;
  }

  return $fields;
}

/**
 * Implements hook_field_instance_settings_form().
 */
function chunks_field_instance_settings_form($field, $instance) {
  $settings = $instance['settings'];

  if ($field['type'] == 'chunks') {

    $chunk_types = chunk_types_load();

    $type_options = array();
    foreach($chunk_types as $name => $chunk_type) {
      $type_options[$name] = $chunk_type->title;
    }

    $form['chunk_instances'] = array(
      '#type' => 'fieldset',
      '#title' => t('Chunk instances'),
      'instances' => array(
        '#theme' => 'chunks_instances_form',
        '#field' => $field,
        '#chunk_types' => $chunk_types,
      ),
      'add_instance' => array(
        '#type' => 'fieldset',
        '#title' => t('Add chunk instance'),
        'type' => array(
          '#type' => 'select',
          '#title' => t('Chunk type'),
          '#options' => $type_options,
        ),
        'button' => array(
          '#type' => 'submit',
          '#value' => t('Add'),
          '#limit_validation_errors' => array(
            array('instance', 'settings', 'chunk_instances'),
          ),
          '#submit' => array('chunks_field_add_chunk_instance'),
        ),
      ),
    );
  }

  return $form;
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * This is necessary because $form_state is not available inside
 * hook_field_instance_settings_form().
 */
function chunks_form_field_ui_field_edit_form_alter(&$form, &$form_state) {
  if ($form['#field']['type'] != 'chunks') {
    return;
  }

  $chunk_types = chunk_types_load();
  $settings = $form['#instance']['settings'];

  // Grab chunk instances from form_state if it exists, and from the settings
  // if it does not.
  if (isset($form_state['values'])) {
    $instances = $form_state['values']['instance']['settings']['chunk_instances']['instances'];
  }
  else {
    // Convert instances into an indexed array to make it easier to add new
    // instances.
    $instances = array_values($settings['chunk_instances']);
  }

  // Sort chunk instances by weight.
  uasort($instances, 'drupal_sort_weight');

  // Store the total number of instances
  $instance_num = count($instances);

  // Create the chunk_instances wrapper element.
  $instances_wrapper = drupal_html_id('chunk-instances');
  $form['instance']['settings']['chunk_instances']['#prefix'] = '<div id="' . $instances_wrapper . '">';
  $form['instance']['settings']['chunk_instances']['#suffix'] = '</div>';

  // Ajaxify the add instance button.
  $form['instance']['settings']['chunk_instances']['add_instance']['button']['#ajax'] = array(
    'callback' => 'chunks_field_retrieve_chunk_instances',
    'wrapper' => $instances_wrapper,
  );

  // Build chunk instance form elements.
  foreach ($instances as $delta => $instance) {
    $chunk_type = $chunk_types[$instance['type']];

    $in_use = isset($instance['name']) && chunks_instance_in_use($instance['name'], $form['#instance']);

    // Provide defaults for any missing settings.
    $instance['settings'] = isset($instance['settings']) ? $instance['settings'] : array();
    $instance['settings'] += $chunk_type->default_type_settings;

    $instance_form = $chunk_type->getTypeSettingsForm($instance, $delta, $form, $form_state);

    // Provide the preview_on_client setting for any client-themeable chunk
    // type.
    if ($chunk_type->client_themeable) {
      $instance_form['preview_on_client'] = array(
        '#type' => 'checkbox',
        '#title' => t('Preview this chunk instance using the client-side, JavaScript theme implementation.'),
        '#default_value' => isset($instance['settings']['preview_on_client']) ? $instance['settings']['preview_on_client'] : 0,
        '#description' => t('<strong>WARNING:</strong> This bypasses Drupal\'s server-side theming and input filtering processes when previewing an edited chunk. Any potential security vulnerability is mitigated by the fact that unfiltered html is only displayed to the user currently editing, and thus supplying the data with which to generate the html.'),
      );
    }

    // Add the current instance to the form.
    $form['instance']['settings']['chunk_instances']['instances'][$delta] = array(
      'label' => array(
        '#type' => 'textfield',
        '#title' => t('Label'),
        '#size' => 21,
        '#default_value' => isset($instance['label']) ? $instance['label'] : '',
        '#required' => TRUE,
        '#description' => t('The label which will be displayed to the user.'),
      ),
      'name' => array(
        '#type' => 'machine_name',
        '#title' => 'Machine name',
        '#size' => 21,
        '#default_value' => isset($instance['name']) ? $instance['name'] : '',
        '#machine_name' => array(
          'exists' => 'chunks_instance_name_exists',
          'source' => array('instance', 'settings', 'chunk_instances', 'instances', $delta, 'label'),
        ),
      ),
      'type' => array(
        '#type' => 'value',
        '#value' => $instance['type'],
      ),
      'type_display' => array(
        '#markup' => $chunk_type->title,
      ),
      'settings' => $instance_form,
      'weight' => array(
        '#type' => 'weight',
        '#title' => t('Weight for row @number', array('@number' => $delta + 1)),
        '#delta' => $instance_num,
        '#default_value' => $delta,
        '#attributes' => array(
          'class' => array($form['#field']['field_name'] . '-chunk-instances-order'),
        ),
      ),
      'remove' => array(
        '#type' => 'submit',
        '#value' => t('Remove'),
        '#limit_validation_errors' => array(
          array('instance', 'settings', 'chunk_instances', 'add_instance'),
        ),
        '#submit' => array('chunks_field_remove_chunk_instance'),
        '#name' => 'remove-chunk-instance-' . $delta,
        '#ajax' => array(
          'callback' => 'chunks_field_retrieve_chunk_instances',
          'wrapper' => $instances_wrapper,
        ),
        '#access' => !$in_use,
      ),
      'remove_and_delete' => array(
        '#type' => 'submit',
        '#value' => t('Remove'),
        '#limit_validation_errors' => array(
          array('instance', 'settings', 'chunk_instances', 'add_instance'),
        ),
        '#submit' => array('chunks_field_remove_chunk_instance'),
        '#name' => 'remove-and-delete-chunk-instance-' . $delta,
        '#access' => $in_use,
      ),
      '#in_use' => $in_use,
      '#instance_name' => isset($instance['name']) ? $instance['name'] : ''
    );

    // Make the chunk instance form easier to reference.
    $chunk_instance_form = &$form['instance']['settings']['chunk_instances']['instances'][$delta];

    if ($in_use) {
      $chunk_instance_form['name']['#value'] = $instance['name'];
      $chunk_instance_form['name']['#disabled'] = TRUE;
    }

    // Add #limit_validation_errors entries for each instance except for the
    // current one.
    for ($i = 0; $i < $instance_num; $i++) {
      if ($i !== $delta) {
        $chunk_instance_form['remove']['#limit_validation_errors'][] = array('instance', 'settings', 'chunk_instances', 'instances', $i);
        $chunk_instance_form['remove_and_delete']['#limit_validation_errors'][] = array('instance', 'settings', 'chunk_instances', 'instances', $i);
      }
    }
  }

  // Prepend submit handler for processing chunk instances before saving to
  // the field instance settings.
  if (!empty($form['#submit'])) {
    array_unshift($form['#submit'], 'chunks_field_chunk_instances_presave');
  }
  else {
    $form['#submit'] = array('chunks_field_chunk_instances_presave');
  }
}

/**
 * #ajax callback returning the chunk_instances element from
 * chunks_field_instance_settings_form().
 *
 * @see chunks_field_instance_settings_form().
 * @see chunks_form_field_ui_field_edit_form_alter().
 */
function chunks_field_retrieve_chunk_instances($form, $form_state) {
  return drupal_render($form['instance']['settings']['chunk_instances']);
}

/**
 * #empty callback for chunk instance machine name field.
 */
function chunks_instance_name_exists($value, $element, &$form_state) {
  // Retrieve the delta of the current instance.
  end($element['#array_parents']);
  $delta = prev($element['#array_parents']);

  // Return true if the name exists in the current form_state.
  foreach ($form_state['values']['instance']['settings']['chunk_instances']['instances'] as $compare_delta => $instance) {
    if ($value === $instance['name'] && $delta !== $compare_delta) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * #submit callback for adding new chunk instances to a field instance.
 */
function chunks_field_add_chunk_instance($form, &$form_state) {

  // Make sure the chunk_instances array exists.
  if (!isset($form_state['values']['instance']['settings']['chunk_instances']['instances'])) {
    $form_state['values']['instance']['settings']['chunk_instances']['instances'] = array();
  }

  $form_state['values']['instance']['settings']['chunk_instances']['instances'][] = array(
    'label' => '',
    'name' => '',
    'type' => $form_state['values']['instance']['settings']['chunk_instances']['add_instance']['type'],
    'weight' => count($form_state['values']['instance']['settings']['chunk_instances']['instances']) + 1,
  );
  $form_state['rebuild'] = TRUE;
}

/**
 * #submit callback for removing chunk instances from a field instance.
 */
function chunks_field_remove_chunk_instance($form, &$form_state) {

  // Retrieve the index of the instance to remove.
  $parents = $form_state['triggering_element']['#array_parents'];
  end($parents);
  $delta = prev($parents);
  $element = $form['instance']['settings']['chunk_instances']['instances'][$delta];
  $field = $form['#field'];
  $field_instance = $form['#instance'];

  // If the instance has data, redirect the user to
  // chunks_chunk_instance_remove_form() so existing chunks can be removed
  // before removing the instance.
  if ($element['#in_use']) {
    $admin_path = _field_ui_bundle_admin_path($field_instance['entity_type'], $field_instance['bundle']);
    $field_name = $field['field_name'];
    $instance_name = $element['#instance_name'];
    drupal_goto("$admin_path/fields/$field_name/chunk_instance_remove/$instance_name");
  }

  // Remove the instance and reset the index.
  unset($form_state['values']['instance']['settings']['chunk_instances']['instances'][$delta]);
  unset($form_state['input']['instance']['settings']['chunk_instances']['instances'][$delta]);
  $form_state['values']['instance']['settings']['chunk_instances']['instances'] = array_values($form_state['values']['instance']['settings']['chunk_instances']['instances']);
  $form_state['input']['instance']['settings']['chunk_instances']['instances'] = array_values($form_state['input']['instance']['settings']['chunk_instances']['instances']);

  $form_state['rebuild'] = TRUE;
}

/**
 * #submit callback for chunks_field_instance_settings_form() which processes
 * chunk instances.
 */
function chunks_field_chunk_instances_presave($form, &$form_state) {
  // Save chunk_instances as an array of chunk instances keyed by the chunk
  // instance name.
  $chunk_instances = array();
  foreach ($form_state['values']['instance']['settings']['chunk_instances']['instances'] as $instance) {
    $chunk_instances[$instance['name']] = $instance;
  }

  // Sort chunk instances by weight.
  uasort($chunk_instances, 'drupal_sort_weight');

  // Save chunk instances in the form_state.
  $form_state['values']['instance']['settings']['chunk_instances'] = $chunk_instances;
}

/**
 * Implements hook_field_widget_info().
 */
function chunks_field_widget_info() {
  return array(
    'chunks_default' => array(
      'label' => t('Chunks'),
      'description' => t('Add and rearrange structured chunks of content.'),
      'field types' => array('chunks'),
      'settings' => array(),
      'behaviors' => array(
        'multiple values' => FIELD_BEHAVIOR_CUSTOM,
        'default value' => FIELD_BEHAVIOR_NONE,
      ),
    ),
  );
}

/**
 * Implements hook_field_widget_form().
 */
function chunks_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  $field_name = $field['field_name'];
  $parents = $element['#field_parents'];

  $id_prefix = implode('-', array_merge($parents, array($field_name)));
  $wrapper_id = $id_prefix . '-chunks-field';

  // Prepare settings array for drupal_add_js.
  $js_settings = array(
    'chunks' => array(
      'fields' => array(
        $field_name => array(
          'unlimited' => $field['cardinality'] == FIELD_CARDINALITY_UNLIMITED,
          'instances' => array(),
        ),
      ),
      'callbacks' => array(
        'initialize' => new stdClass,
        'saveConfig' => new stdClass,
        'restoreConfig' => new stdClass,
        'stagedChunkShown' => new stdClass,
      ),
    ),
  );

  // Retrieve all chunk_types for use later.
  $chunk_types = chunk_types_load();

  // Save chunk types in the form so they can be accessed easily later.
  $form['#chunk_types'] = $chunk_types;

  // Force the list of chunks to be a dictionary instead of an array.
  $js_settings['chunks']['fields'][$field_name]['chunks']['empty'] = '';

  // Process chunk instances.
  $chunk_instances = $instance['settings']['chunk_instances'];
  foreach ($chunk_instances as $name => $chunk_instance) {

    // Allow easy reference to the ChunkType object associated with the
    // instnace.
    $chunk_type = $chunk_types[$chunk_instance['type']];
    $chunk_instance['chunk_type'] = $chunk_type;
    $chunk_instances[$name]['chunk_type'] = $chunk_type;

    // Add default settings.
    $chunk_instance['settings'] = isset($chunk_instance['settings']) ? $chunk_instance['settings'] : array();
    if (isset($chunk_type->default_type_settings)) {
      $chunk_instance['settings'] += $chunk_type->default_type_settings;
    }

    // Store each chunk instance in JavaScript settings.
    $js_settings['chunks']['fields'][$field_name]['instances'][$name] = $chunk_instance;

    // Check the current user's permission to edit the chunk instance.
    if (!user_access('use_' . $chunk_type->name . '_chunk') || !$chunk_type->isAllowed($chunk_instance['settings'], $field, $instance, $langcode)) {

      // Do not add to chunk instances.
      continue;
    }

    // Only store instances in #chunk_instances if the user has access to edit.
    $form['#allowed_chunk_instances'][$field_name][$name] = $chunk_instance;
  }

  // Retrieve items from the field state since they may not exist in the form
  // state due to #limit_validation_errors.
  $field_state = field_form_get_state($parents, $field_name, $langcode, $form_state);
  if (isset($field_state['items'])) {
    $items = $field_state['items'];
  }

  // Set maximum number of items to display.
  if ($field['cardinality'] == FIELD_CARDINALITY_UNLIMITED) {
    $max = $field_state['items_count'] + 1;
  }
  else {
    $max = $field['cardinality'] - 1;
  }

  $path = drupal_get_path('module', 'chunks');

  // Create wrapper element.
  $elements = array(
    '#theme' => 'chunks_field_widget_form',
    '#field_name' => $field_name,
    '#cardinality' => $field['cardinality'],
    '#title' => check_plain($instance['label']),
    '#required' => $instance['required'],
    '#description' => field_filter_xss($instance['description']),
    '#max_delta' => $max,
    '#prefix' => '<div class="chunks-field" field_name="' . $field_name . '" langcode="' . $langcode . '" id="' . $wrapper_id . '">',
    '#suffix' => '</div>',
    '#attached' => array(
      'css' => array(
        $path . '/css/chunks-widget.css',
      ),
      'js' => array(
        $path . '/js/Chunk.js',
        $path . '/js/ChunksField.js',
        $path . '/js/chunks-widget.js',
      ),
    ),
  );

  // If cardinality is unlimited, provide a button at the beginning for
  // prepending chunks.
  if ($field['cardinality'] == FIELD_CARDINALITY_UNLIMITED) {
    $elements['add_before_action'] = array(
      '#type' => 'actions',
      '#attributes' => array(
        'class' => array('add-chunk-action', 'add-chunk-action-before'),
      ),
      'add_before' => array(
        '#type' => 'submit',
        '#name' => $field_name . '-add-before',
        '#value' => t('+ Add here'),
        '#submit' => array('chunks_field_form_submit'),
        '#op' => 'add_before',
        '#limit_validation_errors' => array(array($field_name)),
        '#weight' => 2,
        '#ajax' => array(
          'callback' => 'chunks_field_add_chunk_callback',
          'event' => 'click',
          'progress' => array('type' => 'none')
        ),
      ),
    );
  }

  // Loop through $items and create their interface.
  for ($delta = 0; $delta <= $max; $delta++) {

    $class_prepend = str_replace('_', '-', $field_name) . '-' . $delta . '-';

    // Make a copy of the base element so we can reuse it for each chunk.
    $row = $element;

    // Add custom validation function for each chunk and wrapper for ajax
    // callback.
    $row += array(
      '#element_validate' => array('chunks_field_widget_values_validate'),
      '#chunk_instances' => $chunk_instances,
      '#prefix' => '<div id="' . $field_name . '-' . $delta . '-chunk' . '" class="chunk-wrapper" delta="' . $delta . '">',
      '#suffix' => '</div>'
    );

    // Set some defaults for $chunk_state
    $chunk_state = array(
      'view' => 'instance_selection',
      'configuration' => array(),
    );

    // Retrieve the chunk state from $items.
    if (isset($items[$delta])) {
      $chunk_state = $items[$delta] + $chunk_state;
      // Make some changes to the chunk's state if we're dealing with field
      // values from the database.
      if (!isset($form_state['values']) && isset($chunk_state['data'])) {
        // Retrieve and reset the configuration from $chunk_state['data'].
        $data = unserialize($chunk_state['data']);
        $chunk_state['configuration'][$chunk_state['instance']] = $data['configuration'];
        // Set view to preview.
        $chunk_state['view'] = 'preview';
      }
    }

    // Set correct values if the user has selected an instance.
    if (isset($chunk_state['instance'])) {
      $chunk_state['type'] = $chunk_instances[$chunk_state['instance']]['type'];
      $chunk_state['module'] = $chunk_types[$chunk_state['type']]->module;
    }

    // Add default configuration for each chunk instances' configuration.
    foreach ($chunk_instances as $name => $chunk_instance) {
      if (isset($chunk_state['configuration'][$name])) {
        $chunk_state['configuration'][$name] += $chunk_instance['chunk_type']->default_configuration;
      }
      else {
        $chunk_state['configuration'][$name] = $chunk_instance['chunk_type']->default_configuration;
      }
    }

    // Determine whether this chunk is editable by the current user.
    if (!isset($chunk_state['instance']) ||
        (isset($chunk_state['instance']) && isset($form['#allowed_chunk_instances'][$field_name][$chunk_state['instance']]))) {
      $chunk_state['allowed'] = TRUE;
    }
    else {
      $chunk_state['allowed'] = FALSE;
    }

    if ($delta == $max && $field['cardinality'] === FIELD_CARDINALITY_UNLIMITED) {
      // The last chunk should be hidden. It will appear when adding a new chunk
      // so that the editor will see an immediate response.
      $chunk_state['view'] = 'staged';
    }

    // Save the chunk state to JavaScript settings.
    $row['#attached']['js'][] = array(
      'data' => array(
        'chunks' => array(
          'fields' => array(
            $field_name => array(
              'chunks' => array(
                $delta => $chunk_state,
                'empty' => '',
              ),
            ),
          ),
        ),
      ),
      'type' => 'setting',
    );

    if ($field['cardinality'] > 1 || $field['cardinality'] == FIELD_CARDINALITY_UNLIMITED) {
      // Add input for reordering.
      $row['_weight'] = array(
        '#type' => 'weight',
        '#title' => t('Weight for row @number', array('@number' => $delta + 1)),
        '#delta' => $max,
        '#default_value' => isset($items[$delta]['_weight']) ? $items[$delta]['_weight'] : $delta,
        '#weight' => 100,
      );
    }

    // Include hidden field value for the view to display.
    $row['view'] = array(
      '#type' => 'hidden',
      '#default_value' => $chunk_state['view'],
    );

    // If this chunk is editable by the current user, build the instance
    // selection form element.
    if ($chunk_state['allowed']) {

      $options = array();
      foreach ($form['#allowed_chunk_instances'][$field_name] as $name => $chunk_instance) {
        $options[$name] = check_plain($chunk_instance['label']);
      }

      $row['instance'] = array(
        '#type' => 'radios',
        '#title' => t('I\'m adding...'),
        '#attributes' => array(
          'class' => array('chunk-instance-selection'),
        ),
        '#options' => $options,
        '#default_value' => isset($chunk_state['instance']) ? $chunk_state['instance'] : NULL,
      );
    }
    // If this chunk is not editable by the current user, store the instance in
    // a hidden form element.
    else {
      $row['instance'] = array(
        '#type' => 'hidden',
        '#value' => $chunk_state['instance'],
      );
    }

    $row['type'] = array(
      '#type' => 'hidden',
      '#value' => isset($chunk_state['type']) ? $chunk_state['type'] : '',
    );

    $row['module'] = array(
      '#type' => 'hidden',
      '#value' => isset($chunk_state['type']) ? $chunk_state['type'] : '',
    );

    $row['configuration'] = array(
      '#type' => 'fieldset',
      '#title' => t('Configuration'),
      '#attributes' => array(
        'class' => array($class_prepend . 'configuration'),
      ),
    );

    foreach ($chunk_instances as $name => $chunk_instance) {

      if (isset($chunk_state['instance']) && $chunk_state['instance'] != $name) {
        continue;
      }

      $chunk_type = $chunk_instance['chunk_type'];

      // If we are working with a chunk that already has configuration, use it.
      if (isset($chunk_state['instance'])) {
        $configuration = $chunk_state['configuration'][$name];
      }
      else {
        $configuration = $chunk_type->default_configuration;
      }
      $row['configuration'][$name] = $chunk_type->getForm($configuration, $chunk_instance, $form, $form_state, $field, $instance, $langcode, $items, $delta, $element);
      $row['configuration'][$name] += array(
        '#type' => 'container',
        '#attributes' => array(
          'class' => array(
            $chunk_type->name . '-chunk-type-configuration',
            $name . '-chunk-instance-configuration',
          ),
        ),
        '#access' => isset($form['#allowed_chunk_instances'][$field_name][$name]),
      );
    }

    $row['preview_wrapper'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array($class_prepend . 'preview'),
      ),
      '#after_build' => array('chunks_field_build_preview'),
      '#entity' => $element['#entity'],
      '#entity_type' => $element['#entity_type'],
      '#chunk_instances' => $chunk_instances,
      '#field_name' => $field_name,
      '#langcode' => $langcode,
      '#delta' => $delta,
    );

    $row['chunk_actions'] = array(
      '#type' => 'actions',
      '#attributes' => array(
        'class' => array('chunk-actions'),
      ),
      '#weight' => 1,
    );

    // Handle the remove button differently depending on the cardinality
    // settings.
    if ($field['cardinality'] != FIELD_CARDINALITY_UNLIMITED) {
      $row['chunk_actions']['reset'] = array(
        '#type' => 'submit',
        '#name' => $id_prefix . '-' . $delta . '-reset',
        '#submit' => array('chunks_field_form_submit'),
        '#op' => 'reset',
        '#value' => t('Reset'),
        '#weight' => 1,
        '#limit_validation_errors' => array(array($field_name)),
        '#attributes' => array(
          'class' => array($class_prepend . 'reset-button', 'limited'),
        ),
        '#ajax' => array(
          'callback' => 'chunks_field_form_callback',
          'wrapper' => $wrapper_id,
        ),
      );
    }
    else {
      $row['chunk_actions']['remove'] = array(
        '#type' => 'submit',
        '#name' => $id_prefix . '-' . $delta . '-remove',
        '#submit' => array('chunks_field_form_submit'),
        '#op' => 'remove',
        '#value' => t('Remove'),
        '#weight' => 1,
        '#limit_validation_errors' => array(array($field_name)),
        '#attributes' => array(
          'class' => array($class_prepend . 'remove-button', 'unlimited'),
        ),
      );
    }

    // Display chunk editing actions if the current chunk is of an allowed
    // instance.
    if ($chunk_state['allowed']) {

      $row['chunk_actions']['edit'] = array(
        '#type' => 'submit',
        '#name' => $id_prefix . '-' . $delta . '-edit',
        '#value' => t('Edit'),
        '#submit' => array('chunks_field_form_submit'),
        '#op' => 'edit',
        '#limit_validation_errors' => array(array($field_name)),
        '#attributes' => array(
          'class' => array('chunk-edit-button', $class_prepend . 'edit-button'),
        ),
      );

      $row['chunk_actions']['preview'] = array(
        '#type' => 'submit',
        '#name' => $id_prefix . '-' . $delta . '-preview',
        '#value' => t('Preview'),
        '#submit' => array('chunks_field_form_submit'),
        '#op' => 'preview',
        '#limit_validation_errors' => array(array($field_name)),
        '#attributes' => array(
          'class' => array('chunk-preview-button', $class_prepend . 'preview-button'),
        ),
        '#ajax' => array(
          'callback' => 'chunks_field_preview_callback',
          'wrapper' => $field_name . '-' . $delta . '-chunk',
          'event' => 'click',
        ),
      );

      $row['chunk_actions']['cancel'] = array(
        '#type' => 'submit',
        '#name' => $id_prefix . '-' . $delta . '-cancel',
        '#value' => t('Cancel'),
        '#submit' => array('chunks_field_form_submit'),
        '#op' => 'cancel',
        '#weight' => -1,
        '#limit_validation_errors' => array(array($field_name)),
        '#attributes' => array(
          'class' => array('chunk-cancel-button', $class_prepend . 'cancel-button'),
          'style' => 'display: none;',
        ),
      );
    }

    // If cardinality is unlimited, provide buttons for adding new chunks
    // directly after the current chunk.
    if ($field['cardinality'] == FIELD_CARDINALITY_UNLIMITED) {
      $row['add_after_action'] = array(
        '#type' => 'actions',
        '#attributes' => array(
          'class' => array('add-chunk-action', 'add-chunk-action-after'),
        ),
        '#weight' => 2,
        'add_after' => array(
          '#type' => 'submit',
          '#name' => $field_name . '-' . $delta . '-add-after',
          '#value' => t('+ Add here'),
          '#submit' => array('chunks_field_form_submit'),
          '#op' => 'add_after',
          '#limit_validation_errors' => array(array($field_name)),
          '#attributes' => array(
            'class' => array($class_prepend . 'add-after-button'),
          ),
          '#weight' => 2,
          '#ajax' => array(
            'callback' => 'chunks_field_add_chunk_callback',
            'event' => 'click',
            'progress' => array('type' => 'none')
          ),
        ),
      );
    }

    $elements[$delta] = $row;
  }

  // Save JavaScript settings.
  $form['#attached']['js'][] = array(
    'data' => $js_settings,
    'type' => 'setting',
  );

  return $elements;
}

/**
 * #after_build handler for chunk preview elements.
 */
function chunks_field_build_preview($element, &$form_state) {
  $chunk_types = chunk_types_load();
  $chunk_state = drupal_array_get_nested_value($form_state['values'], array_slice($element['#array_parents'], 0, -1));
  if (isset($chunk_state['instance']) && isset($chunk_state['configuration'][$chunk_state['instance']])) {

    // Retrieve chunk type.
    if (empty($chunk_state['type'])) {
      $chunk_state['type'] = $element['#chunk_instances'][$chunk_state['instance']]['type'];
    }
    $chunk_type = $chunk_types[$chunk_state['type']];

    $element['preview'] = array(
      '#theme' => 'chunk',
      '#chunk_type' => $chunk_type,
      '#chunk_configuration' => $chunk_state['configuration'][$chunk_state['instance']] + $chunk_type->default_configuration,
      '#entity' => $element['#entity'],
      '#entity_type' => $element['#entity_type'],
      '#field_name' => $element['#field_name'],
      '#langcode' => $element['#langcode'],
      '#delta' => $element['#delta'],
      '#preview' => TRUE,
    );

    // Add instance settings if they exist.
    if (isset($element['#chunk_instances'][$chunk_state['instance']]['settings'])) {
      $element['preview']['#instance_settings'] = $element['#chunk_instances'][$chunk_state['instance']]['settings'];
    }
    else {
      $element['preview']['#instance_settings'] = array();
    }
  }
  return $element;
}


/**
 * Ajax callback which returns the entire chunks field.
 */
function chunks_field_form_callback($form, $form_state) {
  $triggering_element = $form_state['triggering_element'];
  $parents = $triggering_element['#array_parents'];

  // Retrieve and return the chunk field.
  if ($triggering_element['#op'] == 'add_before') {
    $field = drupal_array_get_nested_value($form, array_slice($parents, 0, -2));
  }
  else {
    $field = drupal_array_get_nested_value($form, array_slice($parents, 0, -3));
  }

  // Force the list of chunks to be a dictionary instead of an array.
  $field['#attached']['js'][] = array(
    'data' => array(
      'chunks' => array(
        'fields' => array(
          $field['#field_name'] => array(
            'chunks' => array(
              'empty' => '',
            ),
          ),
        ),
      ),
    ),
    'type' => 'setting',
  );

  return $field;
}

/**
 * Ajax callback for chunk preview.
 */
function chunks_field_preview_callback($form, $form_state) {
  $triggering_element = $form_state['triggering_element'];
  $parents = $triggering_element['#array_parents'];
  $delta = $parents[count($parents)-3];
  $element = drupal_array_get_nested_value($form, array_slice($parents, 0, -2));

  // Unset weight since it is not being replaced in the markup.
  unset($element['_weight']);

  // Add active class to chunk wrapper.
  $element['#prefix'] = '<div id="' . $element['#field_name'] . '-' . $delta . '-chunk' . '" class="chunk-wrapper active" delta="' . $delta . '">';

  // Use JavaScript settings to inform the application of the loaded preview.
  $element['#attached']['js'][] = array(
    'data' => array(
      'chunks' => array(
        'previewLoaded' => array(
          'fieldName' => $element['#field_name'],
          'delta' => $delta,
        ),
      ),
    ),
    'type' => 'setting',
  );

  return drupal_render($element);
}

/**
 * Ajax callback for adding a new chunk.
 */
function chunks_field_add_chunk_callback($form, $form_state) {
  $triggering_element = $form_state['triggering_element'];
  $parents = $triggering_element['#array_parents'];

  // Retrieve and return the chunk field.
  if ($triggering_element['#op'] == 'add_before') {
    $field = drupal_array_get_nested_value($form, array_slice($parents, 0, -2));
  }
  else {
    $field = drupal_array_get_nested_value($form, array_slice($parents, 0, -3));
  }

  $items = element_children($field);
  $delta = array_pop($items);
  $item = $field[$delta];

  // Build row.
  $item['_weight']['#attributes']['class'] = array($field['#field_name'] . '-delta-order');
  $delta_element = drupal_render($item['_weight']);
  $cells = array(
    array('data' => '', 'class' => array('field-multiple-drag')),
    array('data' => drupal_render($item), 'class' => array('item')),
    array('data' => $delta_element, 'class' => array('delta-order', 'tabledrag-hide')),
  );
  $striping = ($delta + 1) % 2 == 0 ? 'even': 'odd';
  $row = array(
    'data' => $cells,
    'class' => array('draggable', 'chunk-row', 'staged', 'ajax-loaded', $striping),
    'style' => 'display: none;',
    'id' => $field['#field_name'] . '-' . $delta . '-chunk-row',
  );
  $attributes = $row;
  unset($attributes['data']);

  // Build header.
  $header = array(
    array(
      'data' => t('Label'),
      'colspan' => 2,
    ),
    t('Type'),
    t('Settings'),
    t('Order'),
    t('Remove'),
  );

  $ts = tablesort_init($header);

  $output = ' <tr' . drupal_attributes($attributes) . '>';
  $i = 0;
  foreach ($row['data'] as $cell) {
    $cell = tablesort_cell($cell, $header, $ts, $i++);
    $output .= _theme_table_cell($cell);
  }
  $output .= " </tr>\n";

  $commands = array();
  $commands[] = ajax_command_invoke('#' . str_replace('_', '-', $field['#field_name']) . '-values tbody', 'append', array($output));

  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Submission handler for chunk operations.
 */
function chunks_field_form_submit($form, &$form_state) {
  $triggering_element = $form_state['triggering_element'];
  $parents = $triggering_element['#array_parents'];
  $element = drupal_array_get_nested_value($form, array_slice($parents, 0, -2));
  $field_name = $element['#field_name'];
  $language = $element['#language'];
  $element_parents = $element['#field_parents'];
  $depth = $triggering_element['#op'] == 'add_before' ? -2 : -3;
  $items = drupal_array_get_nested_value($form_state['values'], array_slice($parents, 0, $depth));

  // If the triggering element is performing operations on a specific chunk, get
  // the delta and chunk state.
  if ($triggering_element['#op'] != 'add_before') {
    $delta = $parents[count($parents)-3];
    $chunk_state = &$items[$delta];
  }

  // Retrieve the field state.
  $field_state = field_form_get_state($element_parents, $field_name, $language, $form_state);

  // Remove the add_before_action section from items since it does not contain
  // field values.
  if (isset($items['add_before_action'])) {
    unset($items['add_before_action']);
  }

  // Perform operations associated with the triggering_element.
  if (!empty($triggering_element['#op'])) {

    switch ($triggering_element['#op']) {

      case 'edit':
        $chunk_state['view'] = 'configuration';
        break;

      case 'cancel':
      case 'preview':
        $chunk_state['view'] = 'preview';
        break;

      case 'remove':
      case 'reset':

        // Remove the chunk. Should only be run when javascript is disabled.
        unset($items[$delta]);
        $items = array_values($items);

        // decrement the items count and store the items in the field state.
        $field_state['items_count']--;
        break;

      case 'add_after':

        // Set weight for new chunk.
        $weight = $chunk_state['_weight'] + 1;

        // Keep track of highest weight so we can use it to determine the weight
        // of our new staged chunk.
        $highest_weight = $weight;

        foreach ($items as $key => $item) {

          // The staged item is the new item which will be shown to the user.
          if ($item['view'] == 'staged') {
            $items[$key]['view'] = 'instance_selection';
            $items[$key]['_weight'] = (string) $weight;
            continue;
          }

          // Do not continue if the current item has a weight less than $weight.
          if (!isset($items[$key]['_weight']) || $items[$key]['_weight'] < $weight) {
            continue;
          }

          // Increment weights for all items with a weight equal to or higher
          // than the new chunk.
          $items[$key]['_weight']++;

          // Store heighest weight.
          if ($items[$key]['_weight'] > $highest_weight) {
            $highest_weight = $items[$key]['_weight'];
          }

          // Convert weights back to strings.
          $items[$key]['_weight'] = (string) $items[$key]['_weight'];
        }

        // Add empty chunk now that it's weight is unique.
        $items[] = array(
          '_weight' => (string) $highest_weight + 1,
          'chunk' => array(),
        );

        // Increment the items count and store the items in the field state.
        $field_state['items_count']++;
        break;

      case 'add_before':

        $weight = $highest_weight = 0;

        foreach ($items as $key => $item) {

          // Find the staged item.
          if ($item['view'] == 'staged') {
            $staged_item = &$items[$key];
          }

          // Store the lowest weight.
          if (isset($item['_weight']) && $weight >= $item['_weight']) {
            $weight = $item['_weight'] - 1;
          }

          // Store heighest weight.
          if ($items[$key]['_weight'] > $highest_weight) {
            $highest_weight = $items[$key]['_weight'];
          }
        }

        // Change weight and view for staged chunk.
        $staged_item['_weight'] = (string) $weight;
        $staged_item['view'] = 'instance_selection';

        // Create empty chunk with the lowest weight.
        $items[] = array(
          '_weight' => (string) $highest_weight,
          'chunk' => array(),
        );

        // Increment the items count and store the items in the field state.
        $field_state['items_count']++;
        break;
    }
  }

  // Store the items in the field state.
  $field_state['items'] = $items;
  field_form_set_state($element_parents, $field_name, $language, $form_state, $field_state);

  // Save changes to the form state.
  drupal_array_set_nested_value($form_state['values'], array_slice($parents, 0, $depth), $items);
  drupal_array_set_nested_value($form_state['input'], array_slice($parents, 0, $depth), $items);

  $form_state['rebuild'] = TRUE;
}

/**
 * Custom validation for field values in the chunks field widget.
 */
function chunks_field_widget_values_validate($element, &$form_state, $form) {
  $item = drupal_array_get_nested_value($form_state['values'], $element['#array_parents']);
  $triggering_element = $form_state['triggering_element'];

  if (!empty($item['instance']) && $item['view'] != 'removed') {

    $chunk_instance = $element['#chunk_instances'][$item['instance']];

    // Make sure type exists.
    if (empty($item['type'])) {
      $item['type'] = $chunk_instance['type'];
    }

    $chunk_type = $form['#chunk_types'][$item['type']];

    // Do not validate chunk configuration when it is unnecessary, like in the
    // case of removing a chunk or adding a new chunk.
    $validate_configuration = TRUE;

    if (isset($triggering_element['#op'])) {
      switch ($triggering_element['#op']) {
        // When removing or adding chunks, configuration validation is not
        // necessary.
        case 'remove':
        case 'add_after':
        case 'add_before':
          $validate_configuration = FALSE;
          break;
        // Do not validate configuration if the chunk currenty being validated
        // is not the chunk being previewed.
        case 'preview':
          if ($triggering_element['#name'] != $element['chunk_actions']['preview']['#name']) {
            $validate_configuration = FALSE;
          }
          break;
      }
    }

    // Set module value.
    $item['module'] = $chunk_type->module;

    // Validation for chunk type's configuration values.
    if ($validate_configuration) {
      $chunk_type->validateConfiguration($element['configuration'][$item['instance']], $item, $chunk_instance, $form_state, $form);
    }

    // Save changes to the form state.
    drupal_array_set_nested_value($form_state['values'], $element['#array_parents'], $item);
    drupal_array_set_nested_value($form_state['input'], $element['#array_parents'], $item);
  }
}

/**
 * Implements hook_field_formatter_info().
 */
function chunks_field_formatter_info() {
  return array(
    'chunks_default' => array(
      'label' => t('Chunks'),
      'field types' => array('chunks'),
    ),
  );
}

/**
 * Implements hook_field_formatter_view().
 */
function chunks_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();

  foreach ($items as $delta => $item) {

    // Prepare chunk data.
    $item['data'] = unserialize($item['data']);

    // Retrieve chunk instance and type.
    $chunk_instances = $instance['settings']['chunk_instances'];
    $chunk_instance = $chunk_instances[$item['instance']];
    $chunk_type = $item['data']['type'];

    $element[0][$delta] = array(
      '#theme' => 'chunk',
      '#chunk_type' => $chunk_type,
      '#chunk_configuration' => $item['data']['configuration'],
      '#entity' => $entity,
      '#entity_type' => $entity_type,
      '#field_name' => $field['field_name'],
      '#langcode' => $langcode,
      '#delta' => $delta,
      '#preview' => FALSE,
    );

    // Add instance settings if they exist.
    if (isset($chunk_instance['settings'])) {
      $element[0][$delta]['#instance_settings'] = $chunk_instance['settings'];
    }
    else {
      $element[0][$delta]['#instance_settings'] = array();
    }
  }

  return $element;
}

/**
 * Implements hook_field_is_empty().
 */
function chunks_field_is_empty($item, $field) {
  if ($field['type'] == 'chunks') {

    // Make sure we have values for each column.
    if (empty($item['instance']) ||
        empty($item['type']) ||
        empty($item['module']) ||
        empty($item['configuration'][$item['instance']]) ||
        $item['view'] == 'removed') {

      return TRUE;
    }

    $chunk_types = chunk_types_load();

    $chunk_type = $chunk_types[$item['type']];

    // Allow modules to check their chunk type's configuration and determine
    // whether or not it should be considered empty.
    if ($chunk_type->configurationIsEmpty($item['configuration'][$item['instance']], $item, $field)) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Implements hook_field_presave().
 */
function chunks_field_presave($entity_type, $entity, $field, $instance, $langcode, &$items) {
  if ($field['type'] == 'chunks') {


    foreach ($items as $delta => $item) {

      // If we already have data as a serialized array and do not have
      // configuration, unserialize data and set configuration. This handles
      // cases, for instance, where entities are being saved programmatically.
      if (isset($item['data']) && is_string($item['data']) && !isset($item['configuration'])) {
        $data = unserialize($item['data']);
        $configuration = $data['configuration'];
      }
      // We have data from the field widget form.
      else {
        $configuration = $item['configuration'][$item['instance']];
      }

      $chunk_types = chunk_types_load();

      $chunk_type = $chunk_types[$item['type']];

      // Add any missing configuration values from default.
      $configuration = $configuration + $chunk_type->default_configuration;

      if (isset($instance['settings']['chunk_instances'][$item['instance']]['settings'])) {
        $instance_settings = $instance['settings']['chunk_instances'][$item['instance']]['settings'];
      }
      else {
        $instance_settings = array();
      }

      // Add any missing type settings values from the default.
      if (isset($chunk_type->default_type_settings)) {
        $instance_settings = $instance_settings + $chunk_type->default_type_settings;
      }

      // Create and save the data field value since it does not have a
      // corresponding field element.
      $items[$delta]['data'] = array(
        'type' => $chunk_type,
        'configuration' => $configuration,
      );

      // Allow modules to act on chunk data before saving.
      $chunk_type->presave($items[$delta], $entity_type, $entity, $field, $instance, $langcode);

      // Serialize data after other modules have had a chance to alter it.
      $items[$delta]['data'] = serialize($items[$delta]['data']);
    }
  }
}

/**
 * Form constructor for the removal of a chunk isntance from a field instance.
 */
function chunks_chunk_instance_remove_form($form, &$form_state, $field_instance, $chunk_instance) {

  // Build destination path.
  $admin_path = _field_ui_bundle_admin_path($field_instance['entity_type'], $field_instance['bundle']);
  $destination = $admin_path . '/fields/' . $field_instance['field_name'];

  // Make sure this is an actual chunk instance.
  if (!isset($field_instance['settings']['chunk_instances'][$chunk_instance])) {
    drupal_goto($destination);
  }

  $form['entity_type'] = array('#type' => 'value', '#value' => $field_instance['entity_type']);
  $form['field_instance'] = array('#type' => 'value', '#value' => $field_instance);
  $form['chunk_instance'] = array('#type' => 'value', '#value' => $chunk_instance);

  $output = confirm_form($form,
    t('Are you sure you want to delete the %chunk_instance chunk instance from the %field_name field?', array(
      '%chunk_instance' => $chunk_instance,
      '%field_name' => $field_instance['field_name'],
    )),
    $destination,
    t('There are chunks currently using this chunk instance. If you remmove the chunk instance, they will be deleted. This action cannot be undone.'),
    t('Remove'), t('Cancel'),
    'confirm'
  );

  return $output;
}

/**
 * Submission handler for chunks_chunk_instance_remove_form().
 *
 * @TODO: This has the potential to be a pretty expensive operation. Consider
 * making a cron job of it.
 */
function chunks_chunk_instance_remove_form_submit($form, &$form_state) {
  $entity_type = $form_state['values']['entity_type'];
  $field_instance = $form_state['values']['field_instance'];
  $chunk_instance = $form_state['values']['chunk_instance'];

  $deltas = chunks_entities_containing_chunks_of_instance($chunk_instance, $field_instance);
  $entity_ids = array_keys($deltas);
  $entities = entity_load($entity_type, $entity_ids);

  // Remove all chunks of the given chunk instance.
  foreach ($entities as $eid => $entity) {
    $language = field_language($entity_type, $entity, $field_instance['field_name']);
    $items = $entity->{$field_instance['field_name']}[$language];
    foreach ($deltas[$eid] as $delta) {
      unset($items[$delta]);
    }
    $items = array_values($items);
    $entity->{$field_instance['field_name']}[$language] = $items;
    entity_save($entity_type, $entity);
  }

  // Remove the chunk instance.
  unset($field_instance['settings']['chunk_instances'][$chunk_instance]);
  field_update_instance($field_instance);
}

/**
 * Returns HTML for a chunks field widget.
 *
 * Combine multiple chunks into a table with drag-n-drop reordering.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the form element.
 *
 * @ingroup themeable
 */
function theme_chunks_field_widget_form($variables) {
  $element = $variables['element'];
  $output = '';

  $required = !empty($element['#required']) ? theme('form_required_marker', $variables) : '';

  // This chunks field accepts multiple values. Theme the widget as a table with
  // draggable rows.
  if ($element['#cardinality'] > 1 || $element['#cardinality'] == FIELD_CARDINALITY_UNLIMITED) {
    $table_id = drupal_html_id($element['#field_name'] . '_values');
    $table_class = array('field-multiple-table');
    if ($element['#cardinality'] == FIELD_CARDINALITY_UNLIMITED) {
      $table_class[] = 'field-unlimited-table';
    }
    $order_class = $element['#field_name'] . '-delta-order';

    $header = array(
      array(
        'data' => '<label>' . t('!title !required', array('!title' => $element['#title'], '!required' => $required)) . "</label>",
        'colspan' => 2,
        'class' => array('field-label'),
      ),
      t('Order'),
    );
    $rows = array();

    // Sort items according to '_weight' (needed when the form comes back after
    // preview or failed validation)
    $items = array();
    foreach (element_children($element) as $key) {
      if ($key === 'add_before_action') {
        $add_before_action = &$element[$key];
      }
      else {
        $items[] = &$element[$key];
      }
    }
    usort($items, '_field_sort_items_value_helper');

    if (isset($add_before_action)) {
      // Add the add_before_action as a static row, always on top.
      $cells = array(
        array('data' => drupal_render($add_before_action), 'colspan' => 3),
      );
      $rows[] = array(
        'data' => $cells,
        'no_striping' => TRUE,
        'class' => array('add-chunk-action-before-row'),
      );
    }

    // Add the items as table rows.
    foreach ($items as $key => $item) {
      $item['_weight']['#attributes']['class'] = array($order_class);
      $delta_element = drupal_render($item['_weight']);
      $cells = array(
        array('data' => '', 'class' => array('field-multiple-drag')),
        array('data' => drupal_render($item), 'class' => array('item')),
        array('data' => $delta_element, 'class' => array('delta-order')),
      );
      $row_class = array('draggable', 'chunk-row');
      $rows[] = array(
        'data' => $cells,
        'class' => $row_class,
        'id' => $element['#field_name'] . '-' .  $key . '-chunk-row',
      );
      if ($item['view']['#value'] == 'staged') {
        $rows[$key + 1]['class'][] = 'staged';
        $rows[$key + 1]['style'] = 'display: none;';
      }
    }

    $output = '<div class="form-item">';
    $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => $table_id, 'class' => $table_class)));
    $output .= $element['#description'] ? '<div class="description">' . $element['#description'] . '</div>' : '';
    $output .= '</div>';

    drupal_add_tabledrag($table_id, 'order', 'sibling', $order_class);
  }
  // The field cardinality is set to 1. Render a single chunk widget.
  else {
    $output = '<div class="form-item">';
    $output .= '<label>' . t('!title !required', array('!title' => $element['#title'], '!required' => $required)) . "</label>";

    foreach (element_children($element) as $key) {
      $output .= drupal_render($element[$key]);
    }

    $output .= $element['#description'] ? '<div class="description">' . $element['#description'] . '</div>' : '';
    $output .= '</div>';
  }

  return $output;
}

/**
 * Returns HTML for the chunks instances form.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the form element.
 *
 * @ingroup themeable
 */
function theme_chunks_instances_form($variables) {
  $element = $variables['element'];

  $table_id = drupal_html_id($element['#field']['field_name'] . '-chunk-instances');
  $order_class = $element['#field']['field_name'] . '-chunk-instances-order';

  $chunk_types = chunk_types_load();

  $header = array(
    array(
      'data' => t('Label'),
      'colspan' => 2,
    ),
    t('Type'),
    t('Settings'),
    t('Order'),
    t('Remove'),
  );
  $rows = array();

  foreach (element_children($element) as $key) {

    if (count(element_children($element[$key]['settings'])) > 0) {
      $settings = array(
        '#type' => 'fieldset',
        '#title' => t('%type chunk type settings', array('%type' => $chunk_types[$element[$key]['type']['#value']]->title)),
        '#attributes' => array(
          'class' => array('collapsible', 'collapsed'),
        ),
        '#attached' => array(
          'js' => array(
            'misc/form.js',
            'misc/collapse.js'
          ),
        ),
        'settings' => $element[$key]['settings'],
      );
      if ($element[$key]['#in_use']) {
        $settings['#suffix'] = '<div class="description">' . t('This chunk instances is being used by chunks on this field. Some settings may be unavailable.') . "</div>\n";
      }
    }
    else {
      $settings = $element[$key]['settings'];
    }

    $cells = array(
      array(
        'class' => array('chunk-instance-drag', 'fixed-small'),
      ),
      array(
        'class' => array('fixed-large'),
        'data' => drupal_render($element[$key]['label']) . drupal_render($element[$key]['name']),
      ),
      array(
        'class' => array('fixed-medium'),
        'data' => drupal_render($element[$key]['type_display']),
      ),
      array(
        'class' => array('fluid'),
        'data' => drupal_render($settings),
      ),
      array(
        'class' => array('fixed-medium'),
        'data' => drupal_render($element[$key]['weight']),
      ),
      array(
        'class' => array('fixed-medium'),
        'data' => drupal_render($element[$key]['remove']) . drupal_render($element[$key]['remove_and_delete']),
      ),
    );

    $rows[] = array(
      'data' => $cells,
      'class' => array('draggable'),
    );
  }

  // Add tabledrag functionality.
  drupal_add_tabledrag($table_id, 'order', 'sibling', $order_class);

  // Add inline styles.
  $styles = <<<EOT
.chunk-instances td.fixed-small {
  width: 60px;
}
.chunk-instances td.fixed-medium {
  width: 110px;
}
.chunk-instances td.fixed-large {
  width: 400px;
}
.chunk-instances td .form-item {
  white-space: normal;
}
EOT;

  drupal_add_css($styles, 'inline');

  return theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => $table_id, 'class' => array('chunk-instances'))));
}

