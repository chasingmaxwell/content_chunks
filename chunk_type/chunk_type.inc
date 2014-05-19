<?php

/**
 * @file
 * Provides the ChunkType class.
 */

class ChunkType {

  // The machine-readable name.
  public $name;

  // The human-readable name.
  public $title;

  // An array of default configuration values to be used when populating the
  // default values in the chunk type's configuration form.
  public $default_configuration;

  // The implementing module's name.
  public $module;

  // The file name for the chunk type's template, excluding the extension.
  public $template;

  // The directory path in which to look for the above template file, relative
  // to DRUPAL_ROOT.
  public $template_path;

  // A boolean indicating whether or not this chunk type provides a client-side
  // theme implementation in JavaScript.
  public $client_themeable;

  // The file which must be included before performing actions on this
  // ChunkType object.
  protected $file;

  // The directory path in which to look for the above file, relative to
  // DRUPAL_ROOT.
  protected $file_path;

  // The form_id and function name for loading the the chunk type's
  // configuration form.
  protected $form_id;

  /**
   * Create a new ChunkType.
   *
   * @param $name
   *   The new chunk type's machine-readable name.
   * @param $data
   *   An associative array containing:
   *     - title: The chunk type's human-readable name.
   *     - default configuration: An associative array containing the default
   *       configuration values.
   *     - file: The file which should be included before performing actions on
   *       this CunkType object.
   *     - file path: The directory path in which to look for the above file,
   *       relative to DRUPAL_ROOT.
   *     - template: The file name for the chunk type's template excluding the
   *       extension.
   *     - template path: The directory path in which to look for the above
   *       template file, relative to DRUPAL_ROOT.
   *     - client themeable: A boolean indicating whether or not this chunk type
   *       provides a client-side theme implementation in JavaScript.
   * @param $module
   *   The chunk type's module name.
   */
  function __construct($name, $data, $module) {
    $this->name = $name;
    $this->title = $data['title'];
    $this->default_configuration = $data['default configuration'];
    $this->module = $module;
    $this->form_id = $module . '_' . $name . '_chunk_form';
    if (isset($data['file'])) {
      $this->file = $data['file'];
    }
    if (isset($data['file path'])) {
      $this->file_path = $data['file path'];
    }
    $this->template = $data['template'];
    $this->template_path = $data['template path'];
    $this->client_themeable = $data['client themeable'];
  }

  /**
   * Retrieve the chunk type's configuration form.
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
  public function getForm($form, &$form_state, &$configuration, $settings) {
    if (isset($this->file)) {
      $this->file_include();
    }

    $function = $this->form_id;
    return $function($form, $form_state, $configuration, $settings);
  }

  /**
   * Retrieve the chunk type's instance settings form.
   *
   * @param $field
   *   The parent chunk field's structure.
   * @param $settings
   *   The current settings for the chunk type as an associative array.
   *
   * @return
   *   If the hook exists, return a rendereable array to insert into the field's
   *   instance settings form. Otherwise, FALSE.
   *
   * @see chunks_field_instance_settings_form().
   */
  public function getTypeSettingsForm($field, $settings) {

    $form = array();

    if (isset($this->file)) {
      $this->file_include();
    }

    $function = $this->module . '_' . $this->name . '_chunk_type_settings_form';
    if (function_exists($function)) {
      $form += $function($field, $settings);
    }

    if ($this->client_themeable) {
      $form['preview_on_client'] = array(
        '#type' => 'checkbox',
        '#title' => t('Preview this chunk type using the client-side, JavaScript theme implementation.'),
        '#default_value' => isset($settings['preview_on_client']) ? $settings['preview_on_client'] : 0,
        '#description' => t('<strong>WARNING:</strong> This bypasses Drupal\'s normal theming process when previewing an edited chunk. Preprocessors, processors, and theme hook overrides in php will not be run.'),
      );
    }

    return !empty($form) ? $form : FALSE;
  }

  /**
   * Validate the chunk type's configuration as it exists in the field widget
   * form.
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
  public function validateConfiguration($configuration, &$form_state, $form) {
    if (isset($this->file)) {
      $this->file_include();
    }
    foreach(module_implements($this->name . '_chunk_form_validate') as $module) {
      $function = $module . '_' . $this->name . '_chunk_form_validate';
      $valid = $function($configuration, $form_state, $form);
      if (!$valid) {
        return FALSE;
      }
    }
    return TRUE;
  }

  /**
   * Determine whether the chunk type's configuration is empty.
   *
   * This will determine whether the field item contains data.
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
  public function configurationIsEmpty($configuration, $item, $field) {
    if (isset($this->file)) {
      $this->file_include();
    }
    foreach(module_implements($this->name . '_chunk_is_empty') as $module) {
      $function = $module . '_' . $this->name . '_chunk_is_empty';
      $empty = $function($configuration, $item, $field);
      if ($empty === TRUE) {
        return TRUE;
      }
    }
    return FALSE;
  }

  /**
   * Include the file which provides the necessary hooks for this chunk type.
   */
  public function file_include() {
    require_once DRUPAL_ROOT . '/' . $this->file_path . '/' . $this->file;
  }
}

