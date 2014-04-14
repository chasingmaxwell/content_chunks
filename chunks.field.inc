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
  //  - Add ability to go back to type selection from config.
  //  - Add ability to go back to configuration from preview.
  //  - Add ability to delete chunk from any step.
  $widget = $instance['widget'];
  $settings = $widget['settings'];

  if (isset($form_state['values'][$field['field_name']][$langcode][$delta]['chunk'])) {
    $chunk_state = $form_state['values'][$field['field_name']][$langcode][$delta]['chunk'];
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
        'wrapper' => 'chunk-' . $delta,
      ),
    ),
  );

  /**
   * STEP 1: Provide chunk type selection buttons.
   */
  if (empty($chunk_state['type']) || !is_a($chunk_state['type'], 'ChunkType')) {

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

    // Store whether or not we are configuring this field for the first time or
    // reconfiguring.
    $element['chunk']['reconfigure'] = array(
      '#type' => 'hidden',
      '#value' => isset($chunk_state['reconfigure']) ? $chunk_state['reconfigure'] : FALSE,
    );

    // Provide the chunk configuration form once we know the type.
    if (empty($chunk_state['configuration']) || !empty($chunk_state['reconfigure'])) {
      $element['chunk']['configuration'] = $chunk_state['type']->getForm($form, $form_state, $chunk_state);
      $element['chunk']['configuration_submit'] = array(
        '#type' => 'submit',
        '#name' => 'chunk-' . $delta . '-config',
        '#value' => t('Create chunk'),
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
        '#value' => t('Go back'),
        '#submit' => array('chunks_field_form_submit'),
        '#op' => 'configure_back',
        '#limit_validation_errors' => array(array($field['field_name'])),
        '#ajax' => array(
          'callback' => 'chunks_field_form_callback',
          'wrapper' => 'chunk-' . $delta,
        ),
      );
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
        '#value' => t('Go back'),
        '#submit' => array('chunks_field_form_submit'),
        '#op' => 'preview_back',
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
  // Retrieve and return the current chunk.
  $parents = $form_state['triggering_element']['#parents'];
  $chunk = $form[$parents[0]][$parents[1]][$parents[2]][$parents[3]];
  return $chunk;
}

/**
 * Submission handler for chunk configuration and type selection.
 */
function chunks_field_form_submit($form, &$form_state) {
  $triggering_element = $form_state['triggering_element'];
  $parents = $triggering_element['#parents'];
  $chunk = $form[$parents[0]][$parents[1]][$parents[2]][$parents[3]];
  $chunk_state = &$form_state['values'][$parents[0]][$parents[1]][$parents[2]][$parents[3]];

  // By default, set reconfigure to TRUE so we don't submit the configuration
  // form when we don't intend to.
  $chunk_state['reconfigure'] = TRUE;

  if (!empty($triggering_element['#op'])) {
    switch ($triggering_element['#op']) {
      case 'select_type':
        $chunk_state['type'] = $triggering_element['#chunk_type'];
        break;
      case 'configure':
        $chunk_state['reconfigure'] = FALSE;
        break;
      // @TODO: Add cases for 'configure_back', 'preview_back', and 'remove'.
    }
  }

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

