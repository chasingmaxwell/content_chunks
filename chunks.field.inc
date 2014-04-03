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
      'default_widget' => 'chunks_field_widget_form',
      'default_formatter' => 'chunks_default',
    ),
  );
}

/**
 * Implements hook_field_is_empty().
 */
function chunks_field_is_empty($item, $field) {
}

