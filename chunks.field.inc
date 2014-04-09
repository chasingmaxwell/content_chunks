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
  //  - Load a checkboxes field with some chunk type choices.
  //  - When a chunk type is selected replace the checkboxes with the chunk
  //    type's configuration form.
  //  - When chunk configuration form is completed, display the themed chunk.
  //    (optional?)
  //  - Add validation per chunk through #element_validate.
  //  - Somehow we need to get the "add a new chunk" button to appear in-between
  //    each chunk so that authors don't have to scroll down to the bottom of
  //    the field and rearrange chunks whenever they add a new one.
  if ($instance['widget']['type'] == 'chunks_default') {
    $widget = $instance['widget'];
    $settings = $widget['settings'];

    $element['chunk_wrapper'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'id' => 'chunk-' . $delta,
      ),
    );

    // @TODO: chunk data isn't moving with it's chunk when chunks are
    // rearranged. Find a better way to store chunk data.
    $chunk_data = $form_state["chunk-$delta"];

    // Provide chunk type selection buttons if we don't have one for this chunk.
    if (!isset($chunk_data['chunk_type'])) {
      $chunk_types = chunk_types_load();
      $element['chunk_wrapper']['chunk_types'] = array();
      foreach ($chunk_types as $chunk_type) {
        $element['chunk_wrapper']['chunk_types'][$chunk_type->name] = array(
          '#type' => 'submit',
          '#name' => $chunk_type->name,
          '#value' => $chunk_type->title,
          '#submit' => array('chunks_field_select_type'),
          '#limit_validation_errors' => array(),
          '#ajax' => array(
            'callback' => 'chunks_field_type_selected',
            'wrapper' => 'chunk-' . $delta,
          ),
          '#chunk_type' => $chunk_type,
        );
      }
    }
    // Provide the chunk configuration form once we know the type.
    else if (!isset($chunk_data['configuration'])) {
      $chunk_type = $form_state["chunk-$delta"]['chunk_type'];
      $element['chunk_wrapper']['chunk_configuration'] = array(
        '#prefix' => t('Insert %type chunk configuration form here.', array('%type' => $chunk_type->title)),
        'chunk_config_submit' => array(
          '#type' => 'submit',
          '#value' => t('Create chunk'),
          '#submit' => array('chunks_configuration_submit'),
          // @TODO: validation isn't being limited correctly. Find out what I'm
          // doing wrong.
          '#limit_validation_errors' => array($form_state['field'][$field['field_name']][$langcode]['array_parents']),
          '#ajax' => array(
            'callback' => 'chunks_configuration_callback',
            'wrapper' => 'chunk-' . $delta,
          ),
        ),
      );
    }
    // Display the chunk preview now that we have a fully configured chunk.
    else {
      $chunk_type = $form_state["chunk-$delta"]['chunk_type'];
      $element['chunk_wrapper']['chunk_preview'] = array(
        '#markup' => t('Insert %type chunk preview here.', array('%type' => $chunk_type->title)),
      );
    }
  }

  return $element;
}


/**
 * Ajax callback for chunk type selection.
 */
function chunks_field_type_selected($form, $form_state) {
  $parents = $form_state['triggering_element']['#parents'];
  $delta = $parents[2];
  $chunk_wrapper = $form[$parents[0]][$parents[1]][$parents[2]][$parents[3]];
  $form_state["chunk-$delta"] = array('chunk_type' => $form_state['triggering_element']['#chunk_type']);
  return $form[$parents[0]][$parents[1]][$parents[2]][$parents[3]];
}

/**
 * Submission handler for chunk type selection.
 */
function chunks_field_select_type($form, &$form_state) {
  $parents = $form_state['triggering_element']['#parents'];
  $delta = $parents[2];
  $chunk_wrapper = $form[$parents[0]][$parents[1]][$parents[2]][$parents[3]];
  $form_state["chunk-$delta"] = array('chunk_type' => $form_state['triggering_element']['#chunk_type']);
  $form_state['rebuild'] = TRUE;
}

/**
 * Ajax callback for chunk configuration.
 */
function chunks_configuration_callback($form, $form_state) {
  $parents = $form_state['triggering_element']['#parents'];
  $delta = $parents[2];
  $chunk_wrapper = $form[$parents[0]][$parents[1]][$parents[2]][$parents[3]];
  $configuration = array(); // @TODO: put the configuration values here.
  $form_state["chunk-$delta"]['configuration'] = $configuration;
  return $chunk_wrapper;
}

/**
 * Submission handler for chunk configuration.
 */
function chunks_configuration_submit($form, &$form_state) {
  $parents = $form_state['triggering_element']['#parents'];
  $delta = $parents[2];
  $chunk_wrapper = $form[$parents[0]][$parents[1]][$parents[2]][$parents[3]];
  $configuration = array(); // @TODO: put the configuration values here.
  $form_state["chunk-$delta"]['configuration'] = $configuration;
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
  // Should hook_field_widget_error() be used for this?
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

