<?php

/**
 * @file
 * Provide hooks relevant to the heading chunk type.
 */

/**
 * Implements hook_CHUNK_TYPE_chunk_form().
 */
function chunks_heading_chunk_form($form, &$form_state, &$configuration) {
  $config_form = array();
  $config_form['text'] = array(
    '#type' => 'textfield',
    '#title' => t('Text'),
    '#default_value' => isset($configuration['text']) ? $configuration['text'] : '',
    '#description' => t('Enter the heading text.'),
  );
  // @TODO: restrict options based on the heading before?
  $config_form['level'] = array(
    '#type' => 'select',
    '#title' => t('Level'),
    '#options' => array('2' => 'H2', '3' => 'H3', '4' => 'H4', '5' => 'H5', '6' => 'H6'),
    '#default_value' => isset($configuration['level']) ? $configuration['level'] : '',
  );
  return $config_form;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_form_validate().
 */
function chunks_heading_chunk_form_validate($configuration, &$form_state, $form) {
  if (empty($configuration['text']['#value'])) {
    form_error($configuration['text'], t('Please enter heading text.'));
    return FALSE;
  }
  return TRUE;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_is_empty().
 */
function chunks_heading_chunk_is_empty($configuration, $item, $field) {
  if (empty($configuration['text'])) {
    return TRUE;
  }
  return FALSE;
}
