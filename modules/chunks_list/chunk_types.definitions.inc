<?php

/**
 * @file
 * Provide a definition list chunk type.
 */

/**
 * Implements hook_CHUNK_TYPE_chunk_type_settings_form().
 */
function chunks_list_definitions_chunk_type_settings_form($chunk_instance, $delta, $form, &$form_state) {

  $settings = $chunk_instance['settings'];

  $instance_settings_form = array();

  $formats = array();
  foreach (filter_formats() as $id => $format) {
    $formats[$id] = $format->name;
  }

  $instance_settings_form['format'] = array(
    '#type' => 'select',
    '#title' => t('Text format'),
    '#options' => $formats,
    // By default $settings['format'] is an empty string so the default value
    // can be derived from the first value of $formats which should be the
    // global default.
    '#default_value' => !empty($settings['format']) ? $settings['format'] : reset($formats),
    '#access' => user_access('administer filters'),
  );

  return $instance_settings_form;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_is_allowed().
 */
function chunks_list_definitions_chunk_is_allowed($settings, $field, $instance, $langcode) {
  $format = filter_format_load($settings['format']);
  if (!filter_access($format)) {
    return FALSE;
  }
  return TRUE;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_form().
 */
function chunks_list_definitions_chunk_form(&$configuration, $chunk_instance, $form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {

  $settings = $chunk_instance['settings'];

  $config_form = array();

  $format = filter_format_load($settings['format']);

  $help_text = 'Each new line is either a term or definition. Definition lines are prepended by "-".<br>';
  $help_text .= 'Each term line immediately following a definition line starts a new term/definition group.<br>';
  $help_text .= 'You may include multiple terms or definitions in one term/definition group.';

  $description = t($help_text, array('%format' => $format->name));
  $description .= theme('filter_guidelines', array('format' => $format));

  $config_form['definitions'] = array(
    '#type' => 'textarea',
    '#title' => t('Definitions'),
    '#default_value' => $configuration['definitions'],
    '#value_callback' => 'chunks_list_text_to_definitions',
    '#after_build' => array('chunks_list_definitions_to_text'),
    '#format' => $settings['format'],
    '#description' =>  $description,
  );

  if ($settings['preview_on_client']) {
    $config_form['#attached'] = array(
      'js' => array(
        drupal_get_path('module', 'chunks_list'). '/chunks-list-definitions.js',
      ),
    );
  }

  return $config_form;
}

/**
 * #value_callback for the "definitions" textarea.
 *
 * Breaks one string into an array of term/definition groups.
 */
function chunks_list_text_to_definitions($element, $input = FALSE, $form_state) {
  $return = array();
  $items = $input !== FALSE ? $input : $element['#default_value'];

  if (!is_string($items)) {
    return $items;
  }

  $items = explode("\n", $items);

  $definitions = array();

  $group = 0;
  $new_group = true;
  foreach ((array) $items as $key => $item) {
    if (empty($item)) {
      continue;
    }
    if ($item[0] != '-') {
      if ($new_group) {
        $group = $key;
        $new_group = false;
      }
      $definitions[$group]['terms'][] = trim($item);
    }
    else {
      $definitions[$group]['definitions'][] = trim(substr($item, 1));
      $new_group = true;
    }
  }

  return array_values($definitions);
}

/**
 * #after_build handler for the "definitions" textarea.
 *
 * Turns an array of term/definition groups back to text.
 */
function chunks_list_definitions_to_text($element) {
  $values = array(
    '#default_value' => $element['#default_value'],
    '#value' => $element['#value'],
  );

  foreach ($values as $key => $value) {
    $text = '';
    foreach ($value as $group) {
      foreach ($group as $type => $items) {
        foreach ($items as $item) {
          if (!empty($text)) {
            $text .= "\n";
          }
          $text .= $type == 'terms' ? $item : '-' . $item;
        }
      }
    }
    $element[$key] = $text;
  }

  return $element;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_form_validate().
 */
function chunks_list_definitions_chunk_form_validate($configuration, &$chunk_state, $chunk_instance, &$form_state, $form) {

  // The first line should not be a definition.
  if ($configuration['definitions']['#value'][0] == '-') {
    form_error($configuration['definitions'], t('You can not begin a definition list with a definition.'));
    return FALSE;
  }

  // This should never happen because the chunk type should not be displayed to
  // users who do not have access to the filter format. Still, let's be safe.
  $format = filter_format_load($configuration['definitions']['#format']);

  if (!filter_access($format)) {
    form_error($configuration['definitions'], t('You do not have permission to use the %format text format, which this instance of the definitions chunk type requires.', array('%format' => $format->format)));
    return FALSE;
  }

  return TRUE;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_is_empty().
 */
function chunks_list_definitions_chunk_is_empty($configuration, $item, $field) {
  // If there are any term/definition groups containing data, this chunk is not
  // empty.
  foreach ($configuration['definitions'] as $group) {
    if (!empty($group['terms']) && !empty($group['definitions'])) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_presave().
 */
function chunks_list_definitions_chunk_presave(&$item) {
  // Don't save any empty terms or definitions.
  foreach ($item['data']['configuration']['definitions'] as $index => $group) {
    foreach ($group as $type => $definition_items) {
      foreach ($definition_items as $key => $definition_item) {
        if (empty($definition_item)) {
          unset($item['data']['configuration']['definitions'][$index][$type][$key]);
        }
      }
    }
  }
}
