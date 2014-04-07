<?php

/**
 * @file
 * Provide the chunks field.
 */

/**
 * Implements hook_field_info().
 */
function chunks_field_info() {
  return array(
    'chunks' => array(
      'label' => t('Chunks'),
      'description' => t('Structured and reusable content chunks.'),
      // @TODO: add settings for restricting chunk types per instance.
      'instance_settings' => array(),
      'default_widget' => 'chunks_default',
      'default_formatter' => 'chunks_default',
    ),
  );
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
  //  - Somehow we need to get the "add a new chunk" button to appear in-between
  //    each chunk so that authors don't have to scroll down to the bottom of
  //    the field and rearrange chunks whenever they add a new one.
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
}

