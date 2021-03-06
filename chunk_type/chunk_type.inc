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

  // An array of default chunk type settings values used to populate default
  // values in the chunk type's settings form.
  public $default_type_settings;

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
   *     - default type settings: An associative array containing the default
   *       settings for this chunk type.
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
    $this->default_type_settings = $data['default type settings'];
    $this->module = $module;
    $this->form_id = $module . '_' . $name . '_chunk_form';
    if (isset($data['file'])) {
      $this->file = $data['file'];
    }
    if (isset($data['file path'])) {
      $this->file_path = $data['file path'];
    }
    $this->file_include();
    $this->template = $data['template'];
    $this->template_path = $data['template path'];
    $this->client_themeable = $data['client themeable'];
  }

  /**
   * Allow modules to run additional access checks to determine if a chunk type
   * should be allowed in a given context.
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
  public function isAllowed($settings, $field, $instance, $langcode) {

    // If any implementation of hook_CHUNK_TYPE_chunk_is_allowed returns false,
    // the chunk type is not allowed in this context.
    foreach (module_implements($this->name . '_chunk_is_allowed') as $module) {
      $function = $module . '_' . $this->name . '_chunk_is_allowed';
      if (!$function($settings, $field, $instance, $langcode)) {
        return FALSE;
      }
    }
    return TRUE;
  }

  /**
   * Retrieve the chunk type's configuration form.
   *
   * @param $configuration
   *   An associative array representing the current state of the chunk's
   *   configuration.
   * @param $chunk_instance
   *   An associative array representing an instance of a chunk type connected
   *   to a field instance.
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
   * @param $langcode
   *   The language associated with the parent chunk field's items.
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
  public function getForm(&$configuration, $chunk_instance, $form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {

    $function = $this->form_id;
    return $function($configuration, $chunk_instance, $form, $form_state, $field, $instance, $langcode, $items, $delta, $element);
  }

  /**
   * Retrieve the chunk type's instance settings form.
   *
   * @param $chunk_instance
   *   An associative array representing an instance of a chunk type connected
   *   to a field instance.
   *   - label: The human-readable label for the chunk instance.
   *   - name: The unique machin-readable name for the chunk instance.
   *   - type: The name of the chunk type associated with the chunk instance.
   *   - settings: An associative array of settings for the chunk type instance.
   *     An empty array if no settings exist.
   * @param $delta
   *   The order of this chunk instance in the array of chunk instances.
   * @param $form
   *   The form array as generated by field_ui_field_edit_form(). Pro tip:
   *   $form['#field'] and $form['#instance'] are particularly useful.
   * @param $form_state
   *   The form state array for $form.
   *
   * @return
   *   If the hook exists, return a rendereable array to insert into the field's
   *   instance settings form. Otherwise, FALSE.
   *
   * @see chunks_field_instance_settings_form().
   */
  public function getTypeSettingsForm($chunk_instance, $delta, $form, &$form_state) {

    $instance_settings_form = array();

    $function = $this->module . '_' . $this->name . '_chunk_type_settings_form';
    if (function_exists($function)) {
      $instance_settings_form += $function($chunk_instance, $delta, $form, $form_state);
    }

    return !empty($instance_settings_form) ? $instance_settings_form : FALSE;
  }

  /**
   * Validate the chunk type's configuration as it exists in the field widget
   * form.
   *
   * @param $configuration
   *   The chunk type's configuration form as a renderable array with #values.
   * @param $chunk_state
   *   An associative array representing the current state of the chunk being
   *   validated.
   * @param $chunk_instance
   *   An associative array representing an instance of a chunk type connected
   *   to a field instance.
   *   - label: The human-readable label for the chunk instance.
   *   - name: The unique machin-readable name for the chunk instance.
   *   - type: The name of the chunk type associated with the chunk instance.
   *   - settings: An associative array of settings for the chunk type instance.
   *     An empty array if no settings exist.
   * @param $item_element
   *   The form element for the field item for the current chunk.
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
  public function validateConfiguration($configuration, &$chunk_state, $chunk_instance, $item_element, &$form_state, $form) {

    foreach(module_implements($this->name . '_chunk_form_validate') as $module) {
      $function = $module . '_' . $this->name . '_chunk_form_validate';
      $valid = $function($configuration, $chunk_state, $chunk_instance, $item_element, $form_state, $form);
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
  public function presave(&$item, $entity_type, $entity, $field, $instance, $langcode) {

    foreach(module_implements($this->name . '_chunk_presave') as $module) {
      $function = $module . '_' . $this->name . '_chunk_presave';
      $function($item, $entity_type, $entity, $field, $instance, $langcode);
    }
  }

  /**
   * Include the file which provides the necessary hooks for this chunk type.
   */
  public function file_include() {
    if (isset($this->file)) {
      require_once DRUPAL_ROOT . '/' . $this->file_path . '/' . $this->file;
    }
  }
}

