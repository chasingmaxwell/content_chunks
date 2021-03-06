<?php

/**
 * @file
 * Provide an ordered/unordered list chunk type.
 */

/**
 * Implements hook_CHUNK_TYPE_chunk_type_settings_form().
 */
function chunks_list_list_chunk_type_settings_form($chunk_instance, $delta, $form, &$form_state) {

  $settings = $chunk_instance['settings'];

  $instance_settings_form = array();

  $instance_settings_form['edit_in_place'] = array(
    '#type' => 'checkbox',
    '#title' => t('Edit list items in-place'),
    '#default_value' => $settings['edit_in_place'],
  );

  $instance_settings_form['edit_in_place']['#description'] = t('This setting requires the libraries module (https://drupal.org/project/libraries) and the pen library (https://github.com/sofish/pen).');
  $instance_settings_form['edit_in_place']['#description'] .= '<br>';
  $instance_settings_form['edit_in_place']['#description'] .= t('<strong>WARNING:</strong> This bypasses Drupal\'s server-side theming and input filtering processes when rendering an editable chunk. Any potential security vulnerability is mitigated by the fact that unfiltered html is only displayed to the user currently editing, and thus supplying the data with which to generate the html.');

  // Do not allow the edit_in_place setting to be configured if the pen library
  // does not exist.
  $library = module_exists('libraries') ? libraries_detect('pen') : array('installed' => FALSE);
  if (!$library['installed']) {
    $instance_settings_form['edit_in_place']['#default_value'] = 0;
    $instance_settings_form['edit_in_place']['#value'] = 0;
    $instance_settings_form['edit_in_place']['#disabled'] = TRUE;
  }

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
function chunks_list_list_chunk_is_allowed($settings, $field, $instance, $langcode) {
  $format = filter_format_load($settings['format']);
  if (!filter_access($format)) {
    return FALSE;
  }
  return TRUE;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_form().
 */
function chunks_list_list_chunk_form(&$configuration, $chunk_instance, $form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {

  $settings = $chunk_instance['settings'];

  $config_form = array();

  $config_form['style'] = array(
    '#type' => 'select',
    '#title' => t('Style'),
    '#default_value' => $configuration['style'],
    '#options' => array(
      'ul' => t('Bulleted'),
      'ol' => t('Numbered'),
    ),
    '#description' => t('Bulleted will wrap items in an %ul element. Numbered will wrap items in an %ol element.', array('%ul' => '<ul></ul>', '%ol' => '<ol></ol>')),
  );

  $format = filter_format_load($settings['format']);

  $list_description = t('Enter list items. Each new line is a new item filtered with the %format text format. Empty lines will be discarded.', array('%format' => $format->name));
  $list_description .= theme('filter_guidelines', array('format' => $format));

  $config_form['list'] = array(
    '#type' => 'textarea',
    '#title' => t('List items'),
    '#default_value' => $configuration['list'],
    '#value_callback' => 'chunks_list_text_to_items',
    '#after_build' => array('chunks_list_items_to_text'),
    '#format' => $settings['format'],
    '#description' =>  $list_description,
  );

  // Only allow the edit-in-place functionality if we can load the pen library
  // and edit_in_place is TRUE.
  if ($settings['edit_in_place']) {

    $library = module_exists('libraries') ? libraries_detect('pen') : array('installed' => FALSE);

    if ($library['installed']) {

      $configuration['edit_in_place'] = TRUE;

      // If we have no list items, provide a placeholder item so the
      // contenteditable element is visible.
      $empty = TRUE;
      foreach ($configuration['list'] as $list) {
        if (!empty($list)) {
          $empty = FALSE;
          break;
        }
      }

      $config_form['inplace_items'] = array(
        '#theme' => 'chunk',
        '#chunk_type' => $form['#chunk_types']['list'],
        '#chunk_configuration' => $configuration,
        '#instance_settings' => $settings,
        '#entity' => $element['#entity'],
        '#entity_type' => $element['#entity_type'],
        '#field_name' => $field['field_name'],
        '#langcode' => $langcode,
        '#delta' => $delta,
        '#preview' => TRUE,
        '#after_build' => array('chunks_list_fill_inplace_items'),
        '#attached' => array(
          'libraries_load' => array(
            array('pen'),
          ),
          'js' => array(
            drupal_get_path('module', 'chunks_list') . '/chunks-list-list.js',
          ),
        ),
      );

      // Attach inline styles for edit-in-place placeholders, but only do it
      // once.
      static $css_attached = FALSE;
      if (!$css_attached) {
        $placeholder_styles = <<<EOT
.list-chunk-type-configuration .list-chunk[contenteditable] li:only-child:empty:after {
  content: 'Enter text here...';
}
.list-chunk-type-configuration .list-chunk[contenteditable]:focus li:only-child:after {
  content: '';
}
EOT;
        $config_form['inplace_items']['#attached']['css'][] = array('data' => $placeholder_styles, 'type' => 'inline');

        $css_attached = TRUE;
      }
    }
  }

  if ($settings['preview_on_client']) {
    $config_form['#attached'] = array(
      'js' => array(
        drupal_get_path('module', 'chunks_list'). '/chunks-list-list.js',
      ),
    );
  }

  return $config_form;
}

/**
 * #value_callback for the "list" textarea.
 *
 * Breaks one string into an array of list items.
 */
function chunks_list_text_to_items($element, $input = FALSE, $form_state) {
  $return = array();
  $items = $input !== FALSE ? $input : $element['#default_value'];

  if (is_string($items)) {
    $items = explode("\n", $items);
  }

  // Trim items and add them to the return array if they contain data.
  // '<p><br></p>' is the markup output by Pen when a line is empty.
  foreach ((array) $items as $key => $list_item) {
    $list_item = trim($list_item);
    if (!empty($list_item) && $list_item !== '<p><br></p>') {
      $return[] = $list_item;
    }
  }

  return $return;
}

/**
 * #after_build handler for the "list" textarea.
 *
 * Turns an array of items back to text.
 */
function chunks_list_items_to_text($element) {
  $element['#default_value'] = implode("\n", $element['#default_value']);
  $element['#value'] = implode("\n", $element['#value']);
  return $element;
}

/**
 * #after_build handler for the "inline_list" contenteditable element.
 *
 * Fills contenteditable list items with correct values.
 */
function chunks_list_fill_inplace_items($element) {
  if (isset($form_state['input'])) {
    $input = drupal_array_get_nested_value($form_state['input'], array_slice($element['#array_parents'], 0, -1));
    $element['#chunk_configuration']['list'] = implode("\n", $input);
  }
  return $element;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_form_validate().
 */
function chunks_list_list_chunk_form_validate($configuration, &$chunk_state, $chunk_instance, $item_element, &$form_state, $form) {
  // This should never happen because the chunk type should not be displayed to
  // users who do not have access to the filter format. Still, let's be safe.
  $format = filter_format_load($configuration['list']['#format']);

  if (!filter_access($format)) {
    form_error($configuration['list'], t('You do not have permission to use the %format text format, which this instance of the list chunk type requires.', array('%format' => $format->format)));
    return FALSE;
  }
  return TRUE;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_is_empty().
 */
function chunks_list_list_chunk_is_empty($configuration, $item, $field) {
  // If any list item contains data, this chunk is not empty.
  foreach ($configuration['list'] as $list_item) {
    if (!empty($list_item) || $list_item !== '<br>') {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Implements hook_CHUNK_TYPE_chunk_presave().
 */
function chunks_list_list_chunk_presave(&$item) {
  // Don't save any empty list items.
  $list_items = $item['data']['configuration']['list'];
  foreach ($list_items as $key => $list_item) {
    if (empty($list_item) || $list_item === '<br>') {
      unset($item['data']['configuration']['items'][$key]);
    }
  }
}
