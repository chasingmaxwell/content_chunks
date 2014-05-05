<?php

/**
 * @file
 * Provides the ChunkType class.
 */

class ChunkType {
  public $name;
  public $title;
  public $default_configuration;
  public $module;
  public $form_id;

  function __construct($name, $data, $module) {
    $this->name = $name;
    $this->title = $data['title'];
    $this->default_configuration = $data['default configuration'];
    $this->module = $module;
    $this->form_id = $module . '_' . $name . '_chunk_form';
    $this->file = $data['file'];
    $this->file_path = $data['file path'];
    $this->template = $data['template'];
    $this->template_path = $data['template path'];

    $this->file_include();

    if (!function_exists($this->form_id)) {
      watchdog('chunks', 'The %module module does not provide a configuration form for the %name chunk type.', array('%module' => $this->module, '%name' => $this->name), WATCHDOG_ERROR);
    }
  }

  public function getForm($form, &$form_state, &$configuration) {
    $this->file_include();
    $function = $this->form_id;
    return $function($form, $form_state, $configuration);
  }

  public function getValidation() {
    $this->file_include();
    $function = $this->module . '_' . $this->name . '_chunk_form_validate';
    if (function_exists($function)) {
      return $function;
    }
    return FALSE;
  }

  public function getEmptyCheck() {
    $this->file_include();
    $function = $this->module . '_' . $this->name . '_chunk_is_empty';
    if (function_exists($function)) {
      return $function;
    }
    return FALSE;
  }

  public function file_include() {
    require_once DRUPAL_ROOT . '/' . $this->file_path . '/' . $this->file;
  }
}

