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
    //'block' => t('Drupal block'),
  );
}

/**
 * Implements hook_CHUNK_TYPE_chunk_form().
 */
function chunks_text_chunk_form($form, &$form_state, &$chunk_state) {
  $configuration = array();
  $configuration['text'] = array(
    '#type' => 'textarea',
    '#default_value' => isset($chunk_state['configuration']['text']) ? $chunk_state['configuration']['text'] : '',
  );
  return $configuration;
}

/**
 * Theme implementation for text chunk type.
 */
function theme_chunks_text_chunk($variables) {
  $output = check_plain($variables['text']);
  return $output;
}

