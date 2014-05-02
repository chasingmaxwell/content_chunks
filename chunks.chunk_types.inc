<?php

/**
 * @file
 * Provide configuration forms and theme implementations for default chunk
 * types.
 */

/**
 * Implements hook_CHUNK_TYPE_chunk_form().
 */
function chunks_text_chunk_form($form, &$form_state, &$configuration) {
  $config_form = array();
  $config_form['text'] = array(
    '#type' => 'textarea',
    '#title' => t('Text'),
    '#default_value' => isset($configuration['text']) ? $configuration['text'] : '',
    '#description' => t('Enter some plain text.'),
  );
  return $config_form;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_form_validate().
 */
function chunks_text_chunk_form_validate($configuration, &$form_state, $form) {
  if ($configuration['text']['#value'] == 'chunks') {
    form_error($configuration['text'], t('Enough chunks!'));
    return FALSE;
  }
  return TRUE;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_is_empty().
 */
function chunks_text_chunk_is_empty($configuration, $item, $field) {
  if (empty($configuration['text'])) {
    return TRUE;
  }
  return FALSE;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_form().
 */
function chunks_p_chunk_form($form, &$form_state, &$configuration) {
  $config_form = array();
  $config_form['p'] = array(
    '#type' => 'textarea',
    '#title' => t('Paragraph'),
    '#default_value' => isset($configuration['p']) ? $configuration['p'] : '',
    '#description' => t('Enter a paragraph of text. Line breaks will be discarded.'),
  );
  return $config_form;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_form_validate().
 */
function chunks_p_chunk_form_validate($configuration, &$form_state, $form) {
  if ($configuration['p']['#value'] == 'chunks') {
    form_error($configuration['p'], t('Enough chunks!'));
    return FALSE;
  }
  return TRUE;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_is_empty().
 */
function chunks_p_chunk_is_empty($configuration, $item, $field) {
  if (empty($configuration['p'])) {
    return TRUE;
  }
  return FALSE;
}

