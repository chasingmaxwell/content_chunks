<?php

/**
 * @file
 * Provide the chunks field.
 */

/**
 * Implements hook_field_info().
 */
function chunks_field_info() {

  // Provide the chunks field.
  $fields = array(
    'chunks' => array(
      'label' => t('Chunks'),
      'description' => t('Structured and reusable content chunks.'),
      'default_widget' => 'chunks_default',
      'settings' => array(),
      'instance_settings' => array('chunk_types' => array()),
      'default_formatter' => 'chunks_default',
    ),
  );

  // Populate chunks field instance_settings.
  $chunk_types = chunk_types_load();
  foreach ($chunk_types as $chunk_type) {
    $fields['chunks']['instance_settings']['chunk_types'][] = $chunk_type->name;
  }

  return $fields;
}

/**
 * Implements hook_field_instance_settings_form().
 */
function chunks_field_instance_settings_form($field, $instance) {
  $settings = $instance['settings'];

  if ($field['type'] == 'chunks') {

    $form['chunk_types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Allowed chunk types'),
      '#options' => array(),
      '#default_value' => $settings['chunk_types'],
      '#description' => t('Select the chunk types which can be added through this field instance.'),
    );

    $chunk_types = chunk_types_load();
    foreach ($chunk_types as $chunk_type) {
      $form['chunk_types']['#options'][$chunk_type->name] = $chunk_type->title;
    }
  }

  return $form;
}

/**
 * Implements hook_field_widget_info().
 */
function chunks_field_widget_info() {
  return array(
    'chunks_default' => array(
      'label' => t('Chunks'),
      'description' => t('Add and rearrange structured chunks of content.'),
      'field types' => array('chunks'),
      'settings' => array(), // @TODO: Figure out what should go here.
      'behaviors' => array(
        'multiple values' => FIELD_BEHAVIOR_CUSTOM,
        'default value' => FIELD_BEHAVIOR_NONE,
      ),
    ),
  );
}

/**
 * Implements hook_field_widget_form().
 */
function chunks_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  $field_name = $field['field_name'];
  $parents = $element['#field_parents'];

  $id_prefix = implode('-', array_merge($parents, array($field_name)));
  $wrapper_id = $id_prefix . '-chunks-field';

  // Retrieve all chunk_types for use later.
  $chunk_types = chunk_types_load();

  // Save chunk types in the form so they can be accessed easily later.
  $form['#chunk_types'] = $chunk_types;

  // Filter allowed chunk types by permissions and instance settings.
  $form['#allowed_chunk_types'] = array();
  foreach ($chunk_types as $name => $chunk_type) {
    if ((!isset($instance['settings']['chunk_types'][$name]) || !$instance['settings']['chunk_types'][$name]) ||
      !user_access('use_' . $chunk_type->name . '_chunk')) {
        continue;
      }

    $form['#allowed_chunk_types'][$chunk_type->name] = $chunk_type;
  }

  // Retrieve items from the field state since they may not exist in the form
  // state due to #limit_validation_errors.
  $field_state = field_form_get_state($parents, $field_name, $langcode, $form_state);
  if (isset($field_state['items'])) {
    $items = $field_state['items'];
  }

  // Set maximum number of items to display.
  switch ($field['cardinality']) {
    case FIELD_CARDINALITY_UNLIMITED:
      $max = $field_state['items_count'] + 1;
      break;
    default:
      $max = $field['cardinality'] - 1;
      break;
  }

  // Create wrapper element.
  $elements = array(
    '#theme' => 'chunks_field_widget_form',
    '#field_name' => $field_name,
    '#cardinality' => $field['cardinality'],
    '#title' => check_plain($instance['label']),
    '#required' => $instance['required'],
    '#description' => field_filter_xss($instance['description']),
    '#max_delta' => $max,
    '#prefix' => '<div class="chunks-field" field_name="' . $field_name . '" langcode="' . $langcode . '" id="' . $wrapper_id . '">',
    '#suffix' => '</div>',
    '#attached' => array(
      'css' => array(
        drupal_get_path('module', 'chunks') . '/chunks-widget.css',
      ),
      'js' => array(
        drupal_get_path('module', 'chunks') . '/chunks-widget.js',
      ),
    ),
  );

  // If cardinality is unlimited, provide a button at the beginning for
  // prepending chunks.
  if ($field['cardinality'] == FIELD_CARDINALITY_UNLIMITED) {
    $elements['add_before_action'] = array(
      '#type' => 'actions',
      '#attributes' => array(
        'class' => array('add-chunk-action', 'add-chunk-action-before'),
      ),
      'add_before' => array(
        '#type' => 'submit',
        '#name' => $field_name . '-add-before',
        '#value' => t('+ Add chunk here'),
        '#submit' => array('chunks_field_add_before'),
        '#op' => 'add_before',
        '#limit_validation_errors' => array(array($field_name)),
        '#weight' => 2,
        '#ajax' => array(
          'callback' => 'chunks_field_form_callback',
          'wrapper' => $wrapper_id,
        ),
      ),
    );
  }

  // Loop through $items and create their interface.
  for ($delta = 0; $delta <= $max; $delta++) {

    // Make a copy of the base element so we can reuse it for each chunk.
    $row = $element;

    // Add custom validation function for each chunk and wrapper for ajax
    // callback.
    $row += array(
      '#element_validate' => array('chunks_field_widget_values_validate'),
      '#prefix' => '<div id="' . $field_name . '-' . $delta . '-chunk' . '" class="chunk-wrapper" delta="' . $delta . '">',
      '#suffix' => '</div>'
    );

    // Retrieve the chunk state from $items.
    if (isset($items[$delta])) {
      $chunk_state = $items[$delta];
      // Make some changes to the chunk's state if we're dealing with field
      // values from the database.
      if (!isset($form_state['values']) && isset($chunk_state['data'])) {
        // Retrieve the configuration from $chunk_state['data'].
        $data = unserialize($chunk_state['data']);
        $chunk_state['configuration'][$chunk_state['type']] = $data['configuration'];
        // Set view to preview.
        $chunk_state['view'] = 'preview';
      }
    }
    else {
      $chunk_state = array();
    }

    // Determine whether this chunk is editable by the current user.
    if (!isset($chunk_state['type']) ||
        (isset($chunk_state['type']) && isset($form['#allowed_chunk_types'][$chunk_state['type']]))) {
      $chunk_state['allowed'] = TRUE;
    }
    else {
      $chunk_state['allowed'] = FALSE;
    }

    if ($delta == $max) {
      // The last chunk should be hidden. It will appear when adding a new chunk
      // so that the editor will see an immediate response.
      $chunk_state['view'] = 'staged';
    }

    // Add input for reordering.
    $row['_weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight for row @number', array('@number' => $delta + 1)),
      '#delta' => $max,
      '#default_value' => isset($items[$delta]['_weight']) ? $items[$delta]['_weight'] : $delta,
        '#disabled' => isset($chunk_state['view']) && $chunk_state['view'] == 'staged',
      '#weight' => 100,
    );

    // Include hidden field value for the view to display. This will give us
    // better control over our #states.
    $row['view'] = array(
      '#type' => 'hidden',
      '#default_value' => isset($chunk_state['view']) ? $chunk_state['view'] : 'type_selection',
    );

    // If this chunk is editable by the current user, build the type selection
    // form element.
    if ($chunk_state['allowed']) {
      // Filter available chunk types by allowed chunk types.
      $options = array();
      foreach ($form['#allowed_chunk_types'] as $chunk_type) {
        $options[$chunk_type->name] = $chunk_type->title;
      }

      $row['type'] = array(
        '#type' => 'radios',
        '#title' => t('I\'m adding'),
        '#attributes' => array(
          'class' => array('chunk-type-selection'),
        ),
        '#options' => $options,
        '#default_value' => isset($chunk_state['type']) ? $chunk_state['type'] : NULL,
        '#disabled' => isset($chunk_state['view']) && $chunk_state['view'] == 'staged',
        '#states' => array(
          'visible' => array(
            ':input[name="' . $field_name . '[' . $langcode . '][' . $delta . '][view]"]' => array('value' => 'type_selection'),
          ),
        ),
      );
    }
    // If this chunk is not editable by the current user, store the type in a
    // hidden form element.
    else {
      $row['type'] = array(
        '#type' => 'hidden',
        '#value' => $chunk_state['type'],
      );
    }

    $row['module'] = array(
      '#type' => 'hidden',
      '#value' => isset($chunk_state['type']) ? $chunk_types[$chunk_state['type']]->module : '',
    );

    $row['configuration'] = array(
      '#type' => 'fieldset',
      '#title' => t('Configuration'),
      '#attributes' => array(
        'class' => array(str_replace('_', '-', $field_name) . '-' . $delta . '-configuration'),
      ),
      '#states' => array(
        'invisible' => array(
          ':input[name="' . $field_name . '[' . $langcode . '][' . $delta . '][view]"]' => array('!value' => 'configuration'),
        ),
      ),
    );

    foreach ($chunk_types as $name => $chunk_type) {
      $configuration = isset($chunk_state['type']) && isset($chunk_state['configuration']) ? $chunk_state['configuration'][$chunk_state['type']] : array();
      $row['configuration'][$chunk_type->name] = $chunk_type->getForm($form, $form_state, $configuration);
      $row['configuration'][$chunk_type->name] += array(
        '#type' => 'container',
        '#states' => array(
          'visible' => array(
            ':input[name="' . $field_name . '[' . $langcode . '][' . $delta . '][type]"]' => array('value' => $chunk_type->name),
          ),
        ),
      );
    }

    $row['preview'] = array(
      '#type' => 'container',
      '#states' => array(
        'visible' => array(
          ':input[name="' . $field_name . '[' . $langcode . '][' . $delta . '][view]"]' => array('value' => 'preview'),
        ),
      ),
    );

    if (isset($chunk_state['type']) && isset($chunk_state['configuration'][$chunk_state['type']])) {
      $row['preview'] += array(
        '#theme' => 'chunk',
        '#chunk_type' => $chunk_types[$chunk_state['type']],
        '#chunk_configuration' => $chunk_state['configuration'][$chunk_state['type']],
      );
    }

    $row['chunk_actions'] = array(
      '#type' => 'actions',
      '#attributes' => array(
        'class' => array('chunk-actions'),
      ),
      '#weight' => 1,
      'remove' => array(
        '#type' => 'submit',
        '#name' => $id_prefix . '-' . $delta . '-remove',
        '#value' => t('Remove'),
        '#submit' => array('chunks_field_form_submit'),
        '#op' => 'remove',
        '#weight' => 1,
        '#disabled' => isset($chunk_state['view']) && $chunk_state['view'] == 'staged',
        '#limit_validation_errors' => array(array($field_name)),
        '#attributes' => array(
          'class' => array(str_replace('_', '-', $field_name) . '-' . $delta . '-remove-button'),
        ),
      ),
    );

    // Display chunk editing actions if the current chunk is of an allowed type.
    if ($chunk_state['allowed']) {

      $row['chunk_actions']['edit'] = array(
        '#type' => 'submit',
        '#name' => $id_prefix . '-' . $delta . '-edit',
        '#value' => t('Edit'),
        '#submit' => array('chunks_field_form_submit'),
        '#op' => 'edit',
        '#limit_validation_errors' => array(array($field_name)),
        '#attributes' => array(
          'class' => array('chunk-edit-button', str_replace('_', '-', $field_name) . '-' . $delta . '-edit-button'),
        ),
        '#states' => array(
          'visible' => array(
            ':input[name="' . $field_name . '[' . $langcode . '][' . $delta . '][view]"]' => array('value' => 'preview'),
          ),
        ),
      );

      $row['chunk_actions']['preview'] = array(
        '#type' => 'submit',
        '#name' => $id_prefix . '-' . $delta . '-preview',
        '#value' => t('Preview'),
        '#submit' => array('chunks_field_form_submit'),
        '#op' => 'preview',
        '#limit_validation_errors' => array(array($field_name)),
        '#attributes' => array(
          'class' => array(str_replace('_', '-', $field_name) . '-' . $delta . '-preview-button'),
        ),
        '#ajax' => array(
          'callback' => 'chunks_field_preview_callback',
          'wrapper' => $field_name . '-' . $delta . '-chunk',
        ),
        '#states' => array(
          'visible' => array(
            ':input[name="' . $field_name . '[' . $langcode . '][' . $delta . '][view]"]' => array('value' => 'configuration'),
          ),
        ),
      );

      // Once we have a preview, display a cancel button.
      if (isset($chunk_state['type']) && isset($chunk_state['configuration'][$chunk_state['type']])) {
        $row['chunk_actions']['cancel'] = array(
          '#type' => 'submit',
          '#name' => $id_prefix . '-' . $delta . '-cancel',
          '#value' => t('Cancel'),
          '#submit' => array('chunks_field_form_submit'),
          '#op' => 'cancel',
          '#weight' => -1,
          '#limit_validation_errors' => array(array($field_name)),
          '#attributes' => array(
            'class' => array('chunk-cancel-button', str_replace('_', '-', $field_name) . '-' . $delta . '-cancel-button'),
          ),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $field_name . '[' . $langcode . '][' . $delta . '][view]"]' => array('value' => 'configuration'),
            ),
          ),
        );
      }
    }

    // If cardinality is unlimited, provide buttons for adding new chunks
    // directly after the current chunk.
    if ($field['cardinality'] == FIELD_CARDINALITY_UNLIMITED) {
      $row['add_after_action'] = array(
        '#type' => 'actions',
        '#attributes' => array(
          'class' => array('add-chunk-action', 'add-chunk-action-after'),
        ),
        '#weight' => 2,
        'add_after' => array(
          '#type' => 'submit',
          '#name' => $field_name . '-' . $delta . '-add-after',
          '#value' => t('+ Add chunk here'),
          '#submit' => array('chunks_field_form_submit'),
          '#op' => 'add_after',
          '#limit_validation_errors' => array(array($field_name)),
          '#attributes' => array(
            'class' => array(str_replace('_', '-', $field_name) . '-' . $delta . '-add-after-button'),
          ),
          '#weight' => 2,
          '#ajax' => array(
            'callback' => 'chunks_field_form_callback',
            'wrapper' => $wrapper_id,
          ),
        ),
      );
    }

    $elements[$delta] = $row;
  }

  return $elements;
}


/**
 * Ajax callback for chunk operations.
 */
function chunks_field_form_callback($form, $form_state) {
  $triggering_element = $form_state['triggering_element'];
  $parents = $triggering_element['#array_parents'];
  // Retrieve and return the chunk field.
  switch ($triggering_element['#op']) {
    case 'add_before':
      $field = drupal_array_get_nested_value($form, array_slice($parents, 0, -2));
      break;
    default:
      $field = drupal_array_get_nested_value($form, array_slice($parents, 0, -3));
      break;
  }
  return $field;
}

/**
 * Ajax callback for chunk preview.
 */
function chunks_field_preview_callback($form, $form_state) {
  $triggering_element = $form_state['triggering_element'];
  $parents = $triggering_element['#array_parents'];
  $delta = $parents[count($parents)-3];
  $element = drupal_array_get_nested_value($form, array_slice($parents, 0, -2));
  $item = drupal_array_get_nested_value($form_state['values'], array_slice($parents, 0, -2));

  // Unset weight since it is not being replaced in the markup.
  unset($element['_weight']);

  // Add active class to chunk wrapper.
  $element['#prefix'] = '<div id="' . $element['#field_name'] . '-' . $delta . '-chunk' . '" class="chunk-wrapper active" delta="' . $delta . '">';

  return drupal_render($element);
}

/**
 * Submission handler for chunk operations.
 */
function chunks_field_form_submit($form, &$form_state) {
  $triggering_element = $form_state['triggering_element'];
  $parents = $triggering_element['#array_parents'];
  $element = drupal_array_get_nested_value($form, array_slice($parents, 0, -2));
  $field_name = $element['#field_name'];
  $language = $element['#language'];
  $element_parents = $element['#field_parents'];
  $items = drupal_array_get_nested_value($form_state['values'], array_slice($parents, 0, -3));
  $delta = $parents[count($parents)-3];
  $chunk_state = &$items[$delta];
  $field_state = field_form_get_state($element_parents, $field_name, $language, $form_state);

  // Remove the add_before_action section from items since it does not contain
  // field values.
  if (isset($items['add_before_action'])) {
    unset($items['add_before_action']);
  }

  // Perform operations associated with the triggering_element.
  if (!empty($triggering_element['#op'])) {

    switch ($triggering_element['#op']) {

      case 'edit':
        $chunk_state['view'] = 'configuration';
        break;

      case 'cancel':
      case 'preview':
        $chunk_state['view'] = 'preview';
        break;

      case 'remove':
        // Remove the chunk. Should only be run when javascript is disabled.
        unset($items[$delta]);
        $items = array_values($items);

        // Reset weights since the number of possible values (#delta) has
        // changed and the current values might be out of scope.
        usort($items, '_field_sort_items_helper');
        foreach ($items as $key => $item) {
          if (is_array($items[$key]) && isset($items[$key]['_weight'])) {
            $items[$key]['_weight'] = $key;
          }
        }

        // decrement the items count and store the items in the field state.
        $field_state['items_count']--;
        break;

      case 'add_after':

        // Remove any chunks from the form_state which have been removed on the
        // client side through javascript.
        $items_copy = $items;
        $removed = FALSE;
        foreach ($items_copy as $key => $item) {
          if ($item['view'] == 'removed') {
            // remove the chunk.
            unset($items[$key]);
            $removed = TRUE;

            // decrement the items count and store the items in the field state.
            $field_state['items_count']--;
          }
        }
        if ($removed) {
          $items = array_values($items);
        }


        // Set weight for new chunk.
        $weight = $chunk_state['_weight'] + 1;

        // Increase weights for all items with a weight equal to or higher than
        // the new chunk so the new chunk will have a unique weight.
        foreach ($items as $key => $item) {
          if (!isset($items[$key]['_weight']) || $items[$key]['_weight'] < $weight) {
            continue;
          }
          $items[$key]['_weight']++;
        }

        // Add empty chunk now that it's weight is unique.
        $items[] = array(
          '_weight' => $weight,
          'chunk' => array(),
        );

        // Reset weights since the number of possible values (#delta) has
        // changed and the current values might be out of scope.
        usort($items, '_field_sort_items_helper');
        foreach ($items as $key => $item) {
          if (is_array($items[$key]) && isset($items[$key]['_weight'])) {
            $items[$key]['_weight'] = $key;
          }
        }

        // Increment the items count and store the items in the field state.
        $field_state['items_count']++;
        break;
    }
  }

  // Store the items in the field state.
  $field_state['items'] = $items;
  field_form_set_state($element_parents, $field_name, $language, $form_state, $field_state);

  // Save changes to the form state.
  drupal_array_set_nested_value($form_state['values'], array_slice($parents, 0, -3), $items);
  drupal_array_set_nested_value($form_state['input'], array_slice($parents, 0, -3), $items);

  $form_state['rebuild'] = TRUE;
}

/**
 * Submission handler for adding chunks to the beginning of a chunk field.
 */
function chunks_field_add_before($form, &$form_state) {
  $triggering_element = $form_state['triggering_element'];
  $parents = $triggering_element['#array_parents'];
  $element = drupal_array_get_nested_value($form, array_slice($parents, 0, -2));
  $field_name = $element['#field_name'];
  $language = $element['#language'];
  $element_parents = $element['#field_parents'];
  $field_state = field_form_get_state($element_parents, $field_name, $language, $form_state);
  $items = drupal_array_get_nested_value($form_state['values'], array_slice($parents, 0, -2));

  // Remove the add_before_action section from items since it does not contain
  // field values.
  if (isset($items['add_before_action'])) {
    unset($items['add_before_action']);
  }

  // Remove any chunks from the form_state which have been removed on the
  // client side.
  $items_copy = $items;
  $removed = FALSE;
  foreach ($items_copy as $key => $item) {
    if ($item['view'] == 'removed') {
      // remove the chunk.
      unset($items[$key]);
      $removed = TRUE;

      // decrement the items count and store the items in the field state.
      $field_state['items_count']--;
    }
  }
  if ($removed) {
    $items = array_values($items);
  }

  // Retrieve the lowest weight.
  $weight = 0;
  foreach ($items as $key => $item) {
    if (isset($item['_weight']) && $weight >= $item['_weight']) {
      $weight = $item['_weight'] - 1;
    }
  }

  // Create empty chunk with the lowest weight.
  $items[] = array(
    '_weight' => $weight,
    'chunk' => array(),
  );

  // Reset weights since the number of possible values (#delta) has
  // changed and the current values might be out of scope.
  usort($items, '_field_sort_items_helper');
  foreach ($items as $key => $item) {
    if (is_array($items[$key]) && isset($items[$key]['_weight'])) {
      $items[$key]['_weight'] = $key;
    }
  }

  // Increment the items count.
  $field_state['items_count']++;
  $field_state['items'] = $items;
  field_form_set_state($element_parents, $field_name, $language, $form_state, $field_state);

  // Save changes to the form state.
  drupal_array_set_nested_value($form_state['values'], array_slice($parents, 0, -2), $items);
  drupal_array_set_nested_value($form_state['input'], array_slice($parents, 0, -2), $items);

  $form_state['rebuild'] = TRUE;
}

/**
 * Custom validation for field values in the chunks field widget.
 */
function chunks_field_widget_values_validate($element, &$form_state, $form) {
  $item = drupal_array_get_nested_value($form_state['values'], $element['#array_parents']);
  $triggering_element = $form_state['triggering_element'];

  if (isset($item['type'])) {

    $chunk_type = $form['#chunk_types'][$item['type']];

    // Create and save the data field value since it does not have a corresponding
    // field element.
    $data = array(
      'type' => $chunk_type,
      'configuration' => $item['configuration'][$item['type']],
    );
    $item['data'] = serialize($data);
    $item['module'] = $chunk_type->module;

    // Do not validate chunk configuration when it is unnecessary, like in the
    // case of removing a chunk or adding a new chunk.
    $validate_configuration = TRUE;

    if (isset($triggering_element['#op'])) {
      switch ($triggering_element['#op']) {
        case 'remove':
        case 'add_after':
        case 'add_before':
          $validate_configuration = FALSE;
          break;
        case 'preview':
          if ($triggering_element['#name'] != $element['chunk_actions']['preview']['#name']) {
            $validate_configuration = FALSE;
          }
          break;
      }
    }

    // Validation for chunk type's configuration values.
    if ($validate_configuration && $chunk_validation = $chunk_type->getValidation()) {
      $valid = $chunk_validation($element['configuration'][$chunk_type->name], $form_state, $form);
      if (!$valid) {
        $item['view'] = 'configuration';
      }
    }

    // Save changes to the form state.
    drupal_array_set_nested_value($form_state['values'], $element['#array_parents'], $item);
    drupal_array_set_nested_value($form_state['input'], $element['#array_parents'], $item);
  }
}

/**
 * Implements hook_field_formatter_info().
 */
function chunks_field_formatter_info() {
  return array(
    'chunks_default' => array(
      'label' => t('Chunks'),
      'field types' => array('chunks'),
    ),
    'chunks_p' => array(
      'label' => t('Paragraph'),
      'field types' => array('chunks'),
    ),
  );
}

/**
 * Implements hook_field_formatter_view().
 */
function chunks_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $elements = array();
  foreach ($items as $delta => $item) {
    $elements[$delta] = array(
      '#markup' => theme('chunks_formatter_chunks_default', array('element' => $item, 'field' => $instance)),
    );
  }
  return $elements;
}

/**
 * Theme function to generate Chunks.
 */
function theme_chunks_formatter_chunks_default($element) {
  $output = 'begin chunk output --';
  if (!empty($element)) {
    foreach($element as $key => $value) {
      kpr($element);
      $output .= 'chunk loop';
    }
  }
  $output .= "--end chunk output";
  return $output;
}

/**
 * Implements hook_field_is_empty().
 */
function chunks_field_is_empty($item, $field) {
  if ($field['type'] == 'chunks') {

    // Make sure we have values for each column.
    if (empty($item['type']) ||
        empty($item['module']) ||
        empty($item['data']) ||
        $item['view'] == 'removed') {

      return TRUE;
    }

    // Make sure $data is complete.
    $data = unserialize($item['data']);
    if (empty($data['type']) ||
        empty($data['configuration'])) {

      return TRUE;
    }

    // Allow modules to check their chunk type's configuration and determine
    // whether or not it should be considered empty.
    if ($empty_check = $data['type']->getEmptyCheck()) {
      if ($empty_check($data['configuration'], $item, $field)) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

/**
 * Returns HTML for an individual form element.
 *
 * Combine multiple chunks into a table with drag-n-drop reordering.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the form element.
 *
 * @ingroup themeable
 */
function theme_chunks_field_widget_form($variables) {
  $element = $variables['element'];
  $output = '';

  if ($element['#cardinality'] > 1 || $element['#cardinality'] == FIELD_CARDINALITY_UNLIMITED) {
    $table_id = drupal_html_id($element['#field_name'] . '_values');
    $table_class = array('field-multiple-table');
    if ($element['#cardinality'] == FIELD_CARDINALITY_UNLIMITED) {
      $table_class[] = 'field-unlimited-table';
    }
    $order_class = $element['#field_name'] . '-delta-order';
    $required = !empty($element['#required']) ? theme('form_required_marker', $variables) : '';

    $header = array(
      array(
        'data' => '<label>' . t('!title !required', array('!title' => $element['#title'], '!required' => $required)) . "</label>",
        'colspan' => 2,
        'class' => array('field-label'),
      ),
      t('Order'),
    );
    $rows = array();

    // Sort items according to '_weight' (needed when the form comes back after
    // preview or failed validation)
    $items = array();
    foreach (element_children($element) as $key) {
      if ($key === 'add_before_action') {
        $add_before_action = &$element[$key];
      }
      else {
        $items[] = &$element[$key];
      }
    }
    usort($items, '_field_sort_items_value_helper');

    if (isset($add_before_action)) {
      // Add the add_before_action as a static row, always on top.
      $cells = array(
        array('data' => drupal_render($add_before_action), 'colspan' => 3),
      );
      $rows[] = array(
        'data' => $cells,
        'no_striping' => TRUE,
        'class' => array('add-chunk-action-before-row'),
      );
    }

    // Add the items as table rows.
    foreach ($items as $key => $item) {
      $item['_weight']['#attributes']['class'] = array($order_class);
      $delta_element = drupal_render($item['_weight']);
      $cells = array(
        array('data' => '', 'class' => array('field-multiple-drag')),
        array('data' => drupal_render($item), 'class' => array('item')),
        array('data' => $delta_element, 'class' => array('delta-order')),
      );
      $row_class = array('draggable');
      if ($item['view']['#value'] == 'staged') {
        $row_class[] = 'staged';
      }
      $rows[] = array(
        'data' => $cells,
        'class' => $row_class,
        'id' => $element['#field_name'] . '-' .  $key . '-chunk-row',
      );
    }

    $output = '<div class="form-item">';
    $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => $table_id, 'class' => $table_class)));
    $output .= $element['#description'] ? '<div class="description">' . $element['#description'] . '</div>' : '';
    $output .= '</div>';

    drupal_add_tabledrag($table_id, 'order', 'sibling', $order_class);
  }
  else {
    foreach (element_children($element) as $key) {
      $output .= drupal_render($element[$key]);
    }
  }

  return $output;
}

