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
      'instance_settings' => array('chunk_types' => array()),
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

    $form['chunk_types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Allowed chunk types'),
      '#options' => array(),
      '#default_value' => $settings['chunk_types'],
      '#description' => t('Select the chunk types which can be added through this field instance.'),
    );

    $chunk_types = chunk_types_load();
    foreach ($chunk_types as $chunk_type) {
      $form['chunk_types']['#options'][$chunk_type->name] = $chunk_type->title;
    }
  }

  return $form;
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
      'settings' => array(), // @TODO: Figure out what should go here.
      'behaviors' => array(
        'multiple values' => FIELD_BEHAVIOR_DEFAULT,
        'default value' => FIELD_BEHAVIOR_NONE,
      ),
    ),
  );
}

/**
 * Implements hook_field_widget_form().
 */
function chunks_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  // @TODO:
  //  - Add validation per chunk through #element_validate.
  //  - Somehow we need to get the "add a new chunk" button to appear in-between
  //    each chunk so that authors don't have to scroll down to the bottom of
  //    the field and rearrange chunks whenever they add a new one.
  //  - Use the instance settings and permissions to restrict chunk types.
  //  - Handle the multiple values ourselvse so that we can more easily add
  //    chunks in between others and remove chunks. Look at these two examples:
  //    - field_multiple_value_form() in modules/field/field.form.inc
  //    - file_field_widget_form() in modules/file/file.field.inc
  dpm(get_defined_vars());
  $widget = $instance['widget'];
  $settings = $widget['settings'];

  // Retrieve the state for the current chunk.
  if (isset($form_state['values'])) {
    $parents = $element['#field_parents'] + array($field['field_name'], $langcode, $delta, 'chunk');
    $chunk_state = drupal_array_get_nested_value($form_state['values'], $parents);
    if (is_null($chunk_state)) {
      $chunk_state = array();
    }
  }
  else {
    $chunk_state = array();
  }

  $element['chunk'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => 'chunk-' . $delta,
    ),
  );

  $element['chunk']['actions'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('chunk-actions'),
    ),
    '#weight' => 1,
    'remove' => array(
      '#type' => 'submit',
      '#name' => 'chunk-' . $delta . '-remove',
      '#value' => t('Remove'),
      '#submit' => array('chunks_field_form_submit'),
      '#op' => 'remove',
      '#limit_validation_errors' => array(array($field['field_name'])),
      '#weight' => 1,
      '#ajax' => array(
        'callback' => 'chunks_field_form_callback',
        'wrapper' => 'edit-' . str_replace('_', '-', $field['field_name']),
      ),
    ),
  );

  // Track the chunk view persistently so the form can be rebuilt without
  // affecting the view.
  $element['chunk']['view'] = array(
    '#type' => 'hidden',
    '#value' => isset($chunk_state['view']) ? $chunk_state['view'] : 'type_selection',
  );

  /**
   * STEP 1: Provide chunk type selection buttons.
   */
  if (empty($chunk_state['view']) || $chunk_state['view'] == 'type_selection') {

    // Provide a set of buttons which correspond to chunk types available on
    // this field and which the user has permission to use.
    $chunk_types = chunk_types_load();
    $element['chunk']['type'] = array();
    foreach ($chunk_types as $chunk_type) {
      $element['chunk']['type'][$chunk_type->name] = array(
        '#type' => 'submit',
        '#name' => 'chunk-' . $delta . '-' . $chunk_type->name,
        '#value' => $chunk_type->title,
        '#submit' => array('chunks_field_form_submit'),
        '#op' => 'select_type',
        '#limit_validation_errors' => array(array($field['field_name'])),
        '#ajax' => array(
          'callback' => 'chunks_field_form_callback',
          'wrapper' => 'chunk-' . $delta,
        ),
        '#chunk_type' => $chunk_type,
      );
    }
  }

  /**
   * STEP 2: Provide chunk configuration form.
   */
  else {

    // Store the chunk type so it stays in the form state.
    $element['chunk']['type'] = array(
      '#type' => 'hidden',
      '#value' => $chunk_state['type'],
    );

    // Store old configuration if it exists so it can be restored if we "go
    // back" to the preview view.
    $element['chunk']['old_configuration'] = array(
      '#type' => 'hidden',
      '#value' => isset($chunk_state['old_configuration']) ? $chunk_state['old_configuration'] : '',
    );

    // Provide the chunk configuration form once we know the type.
    if ($chunk_state['view'] == 'configuration') {
      $element['chunk']['configuration'] = $chunk_state['type']->getForm($form, $form_state, $chunk_state);
      $element['chunk']['actions']['configuration_submit'] = array(
        '#type' => 'submit',
        '#name' => 'chunk-' . $delta . '-config',
        '#value' => t('Preview'),
        '#submit' => array('chunks_field_form_submit'),
        '#op' => 'configure',
        '#limit_validation_errors' => array(array($field['field_name'])),
        '#ajax' => array(
          'callback' => 'chunks_field_form_callback',
          'wrapper' => 'chunk-' . $delta,
        ),
      );

      // Provide a "back" button when viewing the configuration page. If
      // we already have configuration for this chunk, the back button should
      // take you to the preview, otherwise it should take you back to the type
      // selection step.
      $element['chunk']['actions']['back'] = array(
        '#type' => 'submit',
        '#name' => 'chunk-' . $delta . '-config-back',
        '#value' => t('Cancel'),
        '#submit' => array('chunks_field_form_submit'),
        '#limit_validation_errors' => array(array($field['field_name'])),
        '#ajax' => array(
          'callback' => 'chunks_field_form_callback',
          'wrapper' => 'chunk-' . $delta,
        ),
      );
      // Set #op correctly depending on where "back" is.
      $element['chunk']['actions']['back']['#op'] = empty($chunk_state['configuration']) ? 'back_to_type' : 'back_to_preview';
    }

    /**
     * STEP 3: Display chunk preview.
     */
    else {

      // Store the chunk configuration so it stays in the form state.
      $element['chunk']['configuration'] = array(
        '#type' => 'hidden',
        '#value' => $chunk_state['configuration'],
      );

      // Display the chunk preview now that we have a fully configured chunk.
      $element['chunk']['preview'] = array(
        '#theme' => 'chunk',
        '#chunk_type' => $chunk_state['type'],
        '#chunk_content' => $chunk_state['type']->view($chunk_state['configuration']),
      );

      $element['chunk']['actions']['back'] = array(
        '#type' => 'submit',
        '#name' => 'chunk-' . $delta . '-config-back',
        '#value' => t('Configure'),
        '#submit' => array('chunks_field_form_submit'),
        '#op' => 'back_to_configuration',
        '#limit_validation_errors' => array(array($field['field_name'])),
        '#ajax' => array(
          'callback' => 'chunks_field_form_callback',
          'wrapper' => 'chunk-' . $delta,
        ),
      );
    }
  }

  return $element;
}


/**
 * Ajax callback for chunk configuration and type selection.
 */
function chunks_field_form_callback($form, $form_state) {
  if ($form_state['triggering_element']['#op'] == 'remove') {
    // Retrieve and return the current chunk.
    $element = drupal_array_get_nested_value($form, array_slice($form_state['triggering_element']['#array_parents'], 0, -5));
  }
  else {
    // Retrieve and return the current chunk.
    $element = drupal_array_get_nested_value($form, array_slice($form_state['triggering_element']['#array_parents'], 0, -2));
  }
  return $element;
}

/**
 * Submission handler for chunk configuration and type selection.
 */
function chunks_field_form_submit($form, &$form_state) {
  $triggering_element = $form_state['triggering_element'];
  $parents = $triggering_element['#array_parents'];
  $delta = $parents[count($parents)-4];
  $items = drupal_array_get_nested_value($form_state['values'], array_slice($parents, 0, -4));
  $chunk_state = &$items[$delta]['chunk'];

  if (!empty($triggering_element['#op'])) {
    switch ($triggering_element['#op']) {
      case 'select_type':
        // Set type to ChunkType object.
        $chunk_state['type'] = $triggering_element['#chunk_type'];
        // Progress to the configuration view.
        $chunk_state['view'] = 'configuration';
        break;
      case 'configure':
        // Progress to the preview view.
        $chunk_state['view'] = 'preview';
        break;
      case 'back_to_type':
        // Unset type and progress to the type selection view.
        unset($chunk_state['type']);
        $chunk_state['view'] = 'type_selection';
        break;
      case 'back_to_preview':
        // Restore old configuration.
        $chunk_state['configuration'] = $chunk_state['old_configuration'];
        unset($chunk_state['old_configuration']);
        // Go back to the preview view.
        $chunk_state['view'] = 'preview';
        break;
      case 'back_to_configuration':
        // Store old configuration.
        $chunk_state['old_configuration'] = $chunk_state['configuration'];
        // Go back to the configuration view.
        $chunk_state['view'] = 'configuration';
        break;
      case 'remove':
        unset($items[$delta]);
        $items = array_values($items);
        $element = drupal_array_get_nested_value($form, array_slice($parents, 0, -3));
        $field_state = field_form_get_state($element['#field_parents'], $element['#field_name'], $element['#language'], $form_state);
        $field_state['items_count']--;
        field_form_set_state($element['#field_parents'], $element['#field_name'], $element['#language'], $form_state, $field_state);
        break;
    }
  }

  // Save changes to the form state.
  drupal_array_set_nested_value($form_state['values'], array_slice($parents, 0, -4), $items);
  drupal_array_set_nested_value($form_state['input'], array_slice($parents, 0, -4), $items);

  $form_state['rebuild'] = TRUE;
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
 * Implements hook_field_validate().
 */
function chunks_field_validate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors) {
  // @TODO: Run each chunk configuration through its module's own validation.
  // Should hook_field_widget_error() be used for this (or #element_validate)?
}

/**
 * Implements hook_field_widget_settings_form().
 * @TODO: decide whether we should control the chunk types available here or
 * through hook_field_instance_settings_form().
 */
function chunks_field_widget_settings_form($field, $instance) {
  $widget = $instance['widget'];
  $settings = $widget['settings'];
}

/**
 * Implements hook_field_is_empty().
 */
function chunks_field_is_empty($item, $field) {
  // @TODO: revisit this after chunks_field_widget_form() is built out to make
  // sure it's still correct.
  if ($field['type'] == 'chunks') {
    if (empty($item['chunks'])) {
      return TRUE;
    }
  }
  return FALSE;
}

