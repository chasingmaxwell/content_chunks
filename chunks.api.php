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
 *   - client themeable: A boolean indicating whether or not the chunk type
 *     provides a client-side theme implementation in JavaScript. If TRUE, a
 *     setting will appear on the chunk field's instance settings to allow the
 *     chunk type in that instance to theme the preview on the client. The
 *     module must also register a theme hook in Javascript with the name
 *     "chunk__CHUNK_TYPE". This theme hook will be called to generate the
 *     preview in lieu of an ajax call. You can find more information about
 *     JavaScript theming in Drupal under the "JavaScript theming" header on
 *     this page: https://drupal.org/node/171213.
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
 * @param $chunk_instance
 *   An associative array representing an instance of a chunk type connected to
 *   a field instance.
 *   - label: The human-readable label for the chunk instance.
 *   - name: The unique machin-readable name for the chunk instance.
 *   - type: The name of the chunk type associated with the chunk instance.
 *   - settings: An associative array of settings for the chunk type instance.
 *     An empty array if no settings exist.
 * @param $delta
 *   The order of the chunk instance in the array of chunk instances.
 *
 * @return
 *   A renderable array to insert into the parent field's instance settings
 *   form.
 *
 * @see chunks_field_instance_settings_form().
 */
function hook_CHUNK_TYPE_chunk_type_settings_form($field, $chunk_instance, $delta) {
  $settings = $chunk_instance['settings'];
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
  * Provides additional access checks to determine whether or not the chunk type
  * should be allowed in the given context.
  *
  * @param $settings
  *   An associative array of settings for the chunk type instance. An empty
  *   array if no settings exist.
  * @param $field
  *   The parent field structure.
  * @param $instance
  *   The parent parent field instance.
  * @param $langcode
  *   The language associated with the parent chunk field's items.
  *
  * @return
  *   TRUE if chunk type is allowed in the given context. Otherwise, FALSE.
  */
function hook_CHUNK_TYPE_chunk_is_allowed($settings, $field, $instance, $langcode) {
  if (!filter_access($settings['format'])) {
    return FALSE;
  }
  return TRUE;
}

/**
 * Provides the configuration form to be inserted into the field widget form for
 * a specific chunk type.
 *
 * This hook is required for each chunk type provided.
 *
 * @param $configuration
 *   An associative array representing the current state of the chunk's
 *   configuration.
 * @param $chunk_instance
 *   An associative array representing an instance of a chunk type connected to
 *   a field instance.
 *   - label: The human-readable label for the chunk instance.
 *   - name: The unique machin-readable name for the chunk instance.
 *   - type: The name of the chunk type associated with the chunk instance.
 *   - settings: An associative array of settings for the chunk type instance.
 *     An empty array if no settings exist.
 * @param $form
 *   The edit form for the entity in which the chunks field exists.
 * @param $form_state
 *   An associative array representing the current state of the entity's edit
 *   form.
 * @param $field
 *   The parent field structure.
 * @param $instance
 *   The parent parent field instance.
 * @param $delta
 *   The order of this chunk in the array of chunks.
 * @param $element
 *   A form element array containing basic properties for the parent field
 *   widget.
 *
 * @return
 *   A renderable array to be inserted into the field's widget form.
 *
 * @see chunks_field_widget_form().
 */
function hook_CHUNK_TYPE_chunk_form(&$configuration, $chunk_instance, $form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  $config_form = array();

  $settings = $chunk_instance['settings'];

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
  * @param $chunk_instance
  *   An associative array representing an instance of a chunk type connected to
  *   a field instance.
  *   - label: The human-readable label for the chunk instance.
  *   - name: The unique machin-readable name for the chunk instance.
  *   - type: The name of the chunk type associated with the chunk instance.
  *   - settings: An associative array of settings for the chunk type instance.
  *     An empty array if no settings exist.
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
function hook_CHUNK_TYPE_chunk_form_validate($configuration, $chunk_instance, &$form_state, $form) {
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
  * Act on chunk data before saving.
  *
  * @param $item
  *   The field data for a single chunk.
  * @param $entity_type
  *   The type of $entity.
  * @param $entity
  *   The entity for the operation.
  * @param $field
  *   The parent chunk field's structure.
  * @param $instance
  *   The instance structure for $field on $entity's bundle.
  * @param $langcode
  *   The language associated with the parent chunk field's items.
  *
  * @see chunks_field_presave().
  */
function hook_CHUNK_TYPE_chunk_presave(&$item, $entity_type, $entity, $field, $instance, $langcode) {
  if ($item['data']['configuration']['text'] == 'chunkaroons') {
    // You meant chunks, silly.
    $item['data']['configuration']['text'] = 'chunks';
  }
}

/**
 * @} End of "addtogroup hooks".
 */

