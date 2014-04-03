<?php

/**
 * @file
 * Provide default chunk types.
 */

/**
 * Implements hook_chunk_types() to provide a simple text chunk.
 */
function chunks_chunk_types() {
  return array('text');
}

/**
 * Implements hook_CHUNK_TYPE_chunk_form().
 */
function chunks_text_chunk_form($form, &$form_state) {
  $form['text'] = array(
    '#type' => 'textarea',
    '#default_value' => '',
  );
  return $form;
}

