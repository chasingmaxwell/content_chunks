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
  // @TODO:
  //  - Add validation per chunk through #element_validate.
  //  - Use the instance settings and permissions to restrict chunk types.

  $field_name = $field['field_name'];
  $parents = $element['#field_parents'];

  $id_prefix = implode('-', array_merge($parents, array($field_name)));
  $wrapper_id = drupal_html_id($id_prefix . '-chunks-field');

  // Retrieve items from the field state since they may not exist in the form
  // state due to #limit_validation_errors.
  $field_state = field_form_get_state($parents, $field_name, $langcode, $form_state);
  if (isset($field_state['items'])) {
    $items = $field_state['items'];
  }

  switch ($field['cardinality']) {
    case FIELD_CARDINALITY_UNLIMITED:
      $max = $field_state['items_count'];
      break;
    default:
      $max = $field['cardinality'] - 1;
      break;
  }

  $elements = array(
    '#theme' => 'chunks_field_widget_form',
    '#field_name' => $field_name,
    '#cardinality' => $field['cardinality'],
    '#title' => check_plain($instance['label']),
    '#required' => $instance['required'],
    '#description' => field_filter_xss($instance['description']),
    '#max_delta' => $max,
    '#prefix' => '<div id="' . $wrapper_id . '">',
    '#suffix' => '</div>',
  );

  if ($field['cardinality'] == FIELD_CARDINALITY_UNLIMITED) {
    $elements['add_before_action'] = array(
      '#type' => 'actions',
      '#attributes' => array(
        'class' => array('chunk-actions', 'chunk-actions-before'),
      ),
      'add_before' => array(
        '#type' => 'submit',
        '#name' => $id_prefix . '-add-before',
        '#value' => t('Add chunk here'),
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

  for ($delta = 0; $delta <= $max; $delta++) {

    // Make a copy of the base element so we can reuse it for each chunk.
    $row = $element;

    // Add input for reordering.
    $row['_weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight for row @number', array('@number' => $delta + 1)),
      '#delta' => $max,
      '#default_value' => isset($items[$delta]['_weight']) ? $items[$delta]['_weight'] : $delta,
      '#weight' => 100,
    );

    // Retrieve the state for the current chunk.
    $chunk_state = isset($items[$delta]['chunk']) ? $items[$delta]['chunk'] : array();

    $row['chunk'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'id' => 'chunk-' . $delta . '-wrapper',
        'class' => array('chunk-wrapper'),
      ),
    );

    $row['chunk']['chunk_actions'] = array(
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
        '#limit_validation_errors' => array(array($field_name)),
        '#weight' => 1,
        '#ajax' => array(
          'callback' => 'chunks_field_form_callback',
          'wrapper' => $wrapper_id,
        ),
      ),
    );

    // If cardinality is unlimited, provide buttons for adding new chunks.
    if ($field['cardinality'] == FIELD_CARDINALITY_UNLIMITED) {

      // Add a button for adding a chunk after the current.
      $row['chunk']['add_after_action'] = array(
        '#type' => 'actions',
        '#attributes' => array(
          'class' => array('add-chunk-action', 'add-chunk-action-after'),
        ),
        '#weight' => 2,
        'add_after' => array(
          '#type' => 'submit',
          '#name' => $id_prefix . '-' . $delta . '-add-after',
          '#value' => t('Add chunk here'),
          '#submit' => array('chunks_field_form_submit'),
          '#op' => 'add_after',
          '#limit_validation_errors' => array(array($field_name)),
          '#weight' => 2,
          '#ajax' => array(
            'callback' => 'chunks_field_form_callback',
            'wrapper' => $wrapper_id,
          ),
        ),
      );

    }

    // Track the chunk view persistently so the form can be rebuilt without
    // affecting the view.
    $row['chunk']['view'] = array(
      '#type' => 'hidden',
      '#value' => isset($chunk_state['view']) ? $chunk_state['view'] : 'type_selection',
    );

    /**
     * STEP 1: Provide chunk type selection buttons.
     */
    if (empty($chunk_state['view']) || $chunk_state['view'] == 'type_selection') {

      // Provide a set of buttons which correspond to chunk types available on
      // this field and which the user has permission to use.
      $chunk_types = chunk_types_load();
      $row['chunk']['type'] = array();
      foreach ($chunk_types as $name => $chunk_type) {

        // Skip chunk types which aren't allowed on this instance or that the
        // use does not have permission to use.
        if (!isset($instance['settings']['chunk_types'][$name]) ||
            !user_access('use_' . $chunk_type->name . '_chunk')) {
          continue;
        }

        $row['chunk']['type'][$chunk_type->name] = array(
          '#type' => 'submit',
          '#name' => $id_prefix . '-' . $delta . '-' . $chunk_type->name,
          '#value' => $chunk_type->title,
          '#submit' => array('chunks_field_form_submit'),
          '#op' => 'select_type',
          '#limit_validation_errors' => array(array($field_name)),
          '#ajax' => array(
            'callback' => 'chunks_field_form_callback',
            'wrapper' => $wrapper_id,
          ),
          '#chunk_type' => $chunk_type,
        );
      }
    }

    /**
     * STEP 2: Provide chunk configuration form.
     */
    else {

      // Store the chunk type so it stays in the form state.
      $row['chunk']['type'] = array(
        '#type' => 'hidden',
        '#value' => $chunk_state['type'],
      );

      // Store old configuration if it exists so it can be restored if we "go
      // back" to the preview view.
      $row['chunk']['old_configuration'] = array(
        '#type' => 'hidden',
        '#value' => isset($chunk_state['old_configuration']) ? $chunk_state['old_configuration'] : '',
      );

      // Provide the chunk configuration form once we know the type.
      if ($chunk_state['view'] == 'configuration') {
        $row['chunk']['configuration'] = $chunk_state['type']->getForm($form, $form_state, $chunk_state);
        $row['chunk']['chunk_actions']['configuration_submit'] = array(
          '#type' => 'submit',
          '#name' => $id_prefix . '-' . $delta . '-config',
          '#value' => t('Preview'),
          '#submit' => array('chunks_field_form_submit'),
          '#op' => 'configure',
          '#limit_validation_errors' => array(array($field_name)),
          '#ajax' => array(
            'callback' => 'chunks_field_form_callback',
            'wrapper' => $wrapper_id,
          ),
        );

        // Provide a "back" button when viewing the configuration page. If
        // we already have configuration for this chunk, the back button should
        // take you to the preview, otherwise it should take you back to the
        // type selection step.
        $row['chunk']['chunk_actions']['back'] = array(
          '#type' => 'submit',
          '#name' => $id_prefix . '-' . $delta . '-config-back',
          '#value' => t('Cancel'),
          '#submit' => array('chunks_field_form_submit'),
          '#limit_validation_errors' => array(array($field_name)),
          '#ajax' => array(
            'callback' => 'chunks_field_form_callback',
            'wrapper' => $wrapper_id,
          ),
        );
        // Set #op correctly depending on where "back" is.
        $row['chunk']['chunk_actions']['back']['#op'] = empty($chunk_state['configuration']) ? 'back_to_type' : 'back_to_preview';
      }

      /**
       * STEP 3: Display chunk preview.
       */
      else {

        // Store the chunk configuration so it stays in the form state.
        $row['chunk']['configuration'] = array(
          '#type' => 'hidden',
          '#value' => $chunk_state['configuration'],
        );

        // Display the chunk preview now that we have a fully configured chunk.
        $row['chunk']['preview'] = array(
          '#theme' => 'chunk',
          '#chunk_type' => $chunk_state['type'],
          '#chunk_content' => $chunk_state['type']->view($chunk_state['configuration']),
        );

        $row['chunk']['chunk_actions']['back'] = array(
          '#type' => 'submit',
          '#name' => $id_prefix . '-' . $delta . '-config-back',
          '#value' => t('Edit'),
          '#submit' => array('chunks_field_form_submit'),
          '#op' => 'back_to_configuration',
          '#limit_validation_errors' => array(array($field_name)),
          '#ajax' => array(
            'callback' => 'chunks_field_form_callback',
            'wrapper' => $wrapper_id,
          ),
        );
      }
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
      $field = drupal_array_get_nested_value($form, array_slice($parents, 0, -3));
      break;
    default:
      $field = drupal_array_get_nested_value($form, array_slice($parents, 0, -5));
      break;
  }
  return $field;
}

/**
 * Submission handler for chunk operations.
 */
function chunks_field_form_submit($form, &$form_state) {
  $triggering_element = $form_state['triggering_element'];
  $parents = $triggering_element['#array_parents'];
  $element = drupal_array_get_nested_value($form, array_slice($parents, 0, -3));
  $field_name = $element['#field_name'];
  $language = $element['#language'];
  $element_parents = $element['#field_parents'];
  $items = drupal_array_get_nested_value($form_state['values'], array_slice($parents, 0, -4));
  $delta = $parents[count($parents)-4];
  $chunk_state = &$items[$delta]['chunk'];
  $field_state = field_form_get_state($element_parents, $field_name, $language, $form_state);

  // Remove the add_before_action section from items since it does not contain
  // field values.
  if (isset($items['add_before_action'])) {
    unset($items['add_before_action']);
  }

  // Perform operations associated with the triggering_element.
  if (!empty($triggering_element['#op'])) {
    switch ($triggering_element['#op']) {
      case 'select_type':
        // Set type to ChunkType object.
        $chunk_state['type'] = $triggering_element['#chunk_type'];
        // Progress to the configuration view.
        $chunk_state['view'] = 'configuration';
        break;

      case 'configure':
        // Progress to the preview view.
        $chunk_state['view'] = 'preview';
        break;

      case 'back_to_type':
        // Unset type and progress to the type selection view.
        unset($chunk_state['type']);
        $chunk_state['view'] = 'type_selection';
        break;

      case 'back_to_preview':
        // Restore old configuration.
        $chunk_state['configuration'] = $chunk_state['old_configuration'];
        unset($chunk_state['old_configuration']);
        // Go back to the preview view.
        $chunk_state['view'] = 'preview';
        break;

      case 'back_to_configuration':
        // Store old configuration.
        $chunk_state['old_configuration'] = $chunk_state['configuration'];
        // Go back to the configuration view.
        $chunk_state['view'] = 'configuration';
        break;

      case 'remove':

        // Remove the chunk.
        unset($items[$delta]);
        $items = array_values($items);

        // Reset weights since the number of possible values (#delta) has
        // changed and the current values might be out of scope.
        usort($items, '_field_sort_items_helper');
        foreach ($items as $delta => $item) {
          if (is_array($items[$delta]) && isset($items[$delta]['_weight'])) {
            $items[$delta]['_weight'] = $delta;
          }
        }

        // Decrement the items count and store the items in the field state.
        $field_state['items_count']--;
        break;

      case 'add_after':

        // Set weight for new chunk.
        $weight = $items[$delta]['_weight'] + 1;

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
  drupal_array_set_nested_value($form_state['values'], array_slice($parents, 0, -4), $items);
  drupal_array_set_nested_value($form_state['input'], array_slice($parents, 0, -4), $items);

  $form_state['rebuild'] = TRUE;
}

/**
 * Submission handler for adding chunks to the beginning of a chunk field.
 */
function chunks_field_add_before($form, &$form_state) {
  $triggering_element = $form_state['triggering_element'];
  $parents = $triggering_element['#array_parents'];
  $items = drupal_array_get_nested_value($form_state['values'], array_slice($parents, 0, -2));

  // Remove the add_before_action section from items since it does not contain
  // field values.
  if (isset($items['add_before_action'])) {
    unset($items['add_before_action']);
  }

  // Go one level up in the form, to the widget's container.
  $element = drupal_array_get_nested_value($form, array_slice($parents, 0, -2));
  $field_name = $element['#field_name'];
  $language = $element['#language'];
  $element_parents = $element['#field_parents'];

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
  $field_state = field_form_get_state($element_parents, $field_name, $language, $form_state);
  $field_state['items_count']++;
  $field_state['items'] = $items;
  field_form_set_state($element_parents, $field_name, $language, $form_state, $field_state);

  // Save changes to the form state.
  drupal_array_set_nested_value($form_state['values'], array_slice($parents, 0, -2), $items);
  drupal_array_set_nested_value($form_state['input'], array_slice($parents, 0, -2), $items);

  $form_state['rebuild'] = TRUE;
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
  );
}

/**
 * Implements hook_field_validate().
 */
function chunks_field_validate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors) {
  // @TODO: Run each chunk configuration through its module's own validation.
  // Should hook_field_widget_error() be used for this (or #element_validate)?
}

/**
 * Implements hook_field_widget_settings_form().
 * @TODO: decide whether we should control the chunk types available here or
 * through hook_field_instance_settings_form().
 */
function chunks_field_widget_settings_form($field, $instance) {
  $widget = $instance['widget'];
  $settings = $widget['settings'];
}

/**
 * Implements hook_field_is_empty().
 */
function chunks_field_is_empty($item, $field) {
  // @TODO: revisit this after chunks_field_widget_form() is built out to make
  // sure it's still correct.
  if ($field['type'] == 'chunks') {
    if (empty($item['chunks'])) {
      return TRUE;
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

    // Add the add_before_action as a static row, always on top.
    $cells = array(
      array('data' => ''),
      array('data' => drupal_render($add_before_action), 'colspan' => 2),
    );
    $rows[] = array(
      'data' => $cells,
      'no_striping' => TRUE,
      'class' => array('odd'),
    );

    // Add the items as table rows.
    foreach ($items as $key => $item) {
      $item['_weight']['#attributes']['class'] = array($order_class);
      $delta_element = drupal_render($item['_weight']);
      $cells = array(
        array('data' => '', 'class' => array('field-multiple-drag')),
        drupal_render($item),
        array('data' => $delta_element, 'class' => array('delta-order')),
      );
      $rows[] = array(
        'data' => $cells,
        'class' => array('draggable'),
      );
    }

    $output = '<div class="form-item">';
    $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => $table_id, 'class' => array('field-multiple-table'))));
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

