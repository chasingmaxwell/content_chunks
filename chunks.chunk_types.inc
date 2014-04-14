<?php

/**
 * @file
 * Provide default chunk types.
 */

/**
 * Implements hook_chunk_types() to provide a simple text chunk.
 */
function chunks_chunk_types() {
  return array(
    'text' => t('Text'),
    'block' => t('Drupal block'),
  );
}

/**
 * Implements hook_CHUNK_TYPE_chunk_form().
 */
function chunks_text_chunk_form($form, &$form_state) {
  $form['configuration']['text'] = array(
    '#type' => 'textarea',
    '#default_value' => '',
  );
  return $form;
}

