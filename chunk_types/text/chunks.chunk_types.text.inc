<?php

/**
 * @file
 * Provide hooks relevant to the text chunk type.
 *
 * @TODO: This is only here for demonstration and testing during development.
 * Delete this chunk type once it is no longer necessary.
 */

/**
 * Implements hook_CHUNK_TYPE_chunk_form().
 */
function chunks_text_chunk_form(&$configuration, $chunk_instance, $form, &$form_state) {
  $settings = $chunk_instance['settings'];
  $config_form = array();
  $config_form['text'] = array(
    '#type' => 'textarea',
    '#title' => t('Text'),
    '#default_value' => $configuration['text'],
    '#description' => t('Enter some plain text.'),
  );

  // Add JavaScript for client-side preview theming.
  if ($settings['preview_on_client']) {
    $config_form['#attached'] = array(
      'js' => array(
        drupal_get_path('module', 'chunks') . '/chunk_types/text/chunks-text.js',
      ),
    );
  }

  return $config_form;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_form_validate().
 */
function chunks_text_chunk_form_validate($configuration, $chunk_instance, &$form_state, $form) {
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
