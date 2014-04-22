<?php

/**
 * @file
 * Provide configuration forms and theme implementations for default chunk
 * types.
 */

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

