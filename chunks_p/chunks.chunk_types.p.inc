<?php

/**
 * @file
 * Provide configuration forms and theme implementations for the Paragraph
 * chunk type.
 */


/**
 * Implements hook_CHUNK_TYPE_chunk_form().
 */
function chunks_p_p_chunk_form($form, &$form_state, &$configuration, $settings) {
  $config_form = array();

  // This field holds the data and interacts with Drupal
  $config_form['p'] = array(
    '#type' => 'hidden',
    '#default_value' => isset($configuration['p']) ? $configuration['p'] : '',
  );
  // This field is the user-facing editor and continually transfers its contents
  // to the p field which is then saved within Drupal.
  $configuration['editor'] = TRUE;
  $config_form['editor'] = array(
    '#theme' => 'chunk',
    '#chunk_type' => $form['#chunk_types']['p'],
    '#chunk_configuration' => $configuration,
    '#type_settings' => $settings,
  );

  // Load editor files
  $config_form['#attached'] = array(
    'css' => array(
      drupal_get_path('module', 'chunks_p') . '/css/pen.css',
    ),
    'js' => array(
      drupal_get_path('module', 'chunks_p') . '/js/pen.js',
      drupal_get_path('module', 'chunks_p') . '/js/pen-init.js',
    ),
  );

  return $config_form;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_form_validate().
 */
function chunks_p_p_chunk_form_validate($configuration, &$form_state, $form) {
  if ($configuration['p']['#value'] == 'chunks') {
    form_error($configuration['p'], t('Enough chunks!'));
    return FALSE;
  }
  return TRUE;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_is_empty().
 */
function chunks_p_p_chunk_is_empty($configuration, $item, $field) {
  if (empty($configuration['p'])) {
    return TRUE;
  }
  return FALSE;
}
