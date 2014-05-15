<?php

/**
 * @file
 * Hooks provided by the chunks module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Defines one or more chunk types that are provided by a module.
 *
 * @return
 *   An associative array whose keys are chunk type names and whose values are
 *   associative arrays, each containing:
 *   - title: A string providing a human-readable title for display in the
 *     administration interface.
 *   - default configuration: An array of default configuration values to be
 *     used when populating the default values in the chunk type's configuration
 *     form.
 *   - file: The file which must be included before performing actions on the
 *     ChunkType object.
 *   - file path: The directory path in which to look for the above file,
 *     relative to DRUPAL_ROOT.
 *   - template: The file name for the chunk type's template, excluding the
 *     extension.
 *   - template path: The directory path in which to look for the above template
 *     file, relative to DRUPAL_ROOT.
 *
 * @see chunk_types_load().
 */
function hook_chunk_types() {
  return array(
    'text' => array(
      'title' => t('Text'),
      'default configuration' => array(
        'text' => '',
      ),
      'file' => 'chunk_types/text/chunks.chunk_types.text.inc',
      'template path' => drupal_get_path('module', 'chunks') . '/chunk_types/text',
    ),
  );
}

/**
 * Provides the chunk type's instance settings form.
 *
 * @param $field
 *   The parent chunk field's structure.
 * @param $settings
 *   The current settings for the chunk type as an associative array.
 *
 * @return
 *   A renderable array to insert into the parent field's instance settings
 *   form.
 *
 * @see chunks_field_instance_settings_form().
 */
function hook_CHUNK_TYPE_chunk_type_settings_form($field, $settings) {
  $form = array();
  $form['default_text'] = array(
    '#type' => 'textarea',
    '#title' => t('Default text'),
    '#default_value' => isset($settings['default_text']) ? $settings['default_text'] : '',
    '#description' => t('Enter the text to use by default if no text is entered.'),
  );
  return $form;
}

/**
 * Provides the configuration form to be inserted into the field widget form for
 * a specific chunk type.
 *
 * This hook is required for each chunk type provided.
 *
 * @param $form
 *   The edit form for the entity in which the chunks field exists.
 * @param $form_state
 *   An associative array representing the current state of the entity's edit
 *   form.
 * @param $configuration
 *   An associative array representing the current state of the chunk's
 *   configuration.
 * @param $settings
 *   An associative array of settings for the chunk type instance. An empty
 *   array if no settings exist.
 *
 * @return
 *   A renderable array to be inserted into the field's widget form.
 *
 * @see chunks_field_widget_form().
 */
function hook_CHUNK_TYPE_chunk_form($form, &$form_state, &$configuration, $settings) {
  $config_form = array();

  $config_form['text'] = array(
    '#type' => 'textarea',
    '#title' => t('Text'),
    '#default_value' => isset($configuration['text']) ? $configuration['text'] : $settings['default_text'],
    '#description' => t('Enter some plain text.'),
  );
  return $config_form;
}

/**
  * Validate the configuration as it exists in the field widget form for a
  * specific chunk type.
  *
  * This hook is optional. However, if a module provides a chunk type without
  * also providing this hook, that chunk type's configuration will always be
  * considered valid.
  *
  * @param $configuration
  *   The chunk type's configuration form as a renderable array with #values.
  * @param $form_state
  *   An associative array representing the current state of the entity's edit
  *   form.
  * @param $form
  *   The edit form for the entity in which the chunks field exists.
  *
  * @return
  *   TRUE if the configuration is valid. Otherwise, FALSE.
  *
  * @see chunks_field_widget_values_validate().
  */
function hook_CHUNK_TYPE_chunk_form_validate($configuration, &$form_state, $form) {
  if ($configuration['text']['#value'] == 'chunks') {
    form_error($configuration['text'], t('Enough chunks!'));
    return FALSE;
  }
  return TRUE;
}

/**
  * Determine whether the configuration is empty for a specific chunk type.
  *
  * This will determine whether the field item contains data.
  *
  * This hook is optional. However, if a module provides a chunk type without
  * also providing this hook, that chunk type's configuration will never be
  * considered empty, and thus, chunks of this type will always be saved to the
  * database regardless of whether or not any configuration was entered.
  *
  * @param $configuration
  *   An associative array representing the current state of the chunk's
  *   configuration.
  * @param $form_state
  *   An associative array representing the current state of the entity's edit
  *   form.
  * @param $form
  *   The edit form for the entity in which the chunks field exists.
  *
  * @return
  *   TRUE if the configuration should be considered empty. Otherwise, FALSE.
  *
  * @see hook_field_is_empty().
  * @see chunks_field_is_empty().
  */
function hook_CHUNK_TYPE_chunk_is_empty($configuration, $item, $field) {
  if (empty($configuration['text'])) {
    return TRUE;
  }
  return FALSE;
}

/**
 * @} End of "addtogroup hooks".
 */

