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
  public $theme_hook;

  function __construct($name, $data, $module) {
    $this->name = $name;
    $this->title = $data['title'];
    $this->default_configuration = $data['default_configuration'];
    $this->module = $module;
    $this->form_id = $module . '_' . $name . '_chunk_form';
    $this->theme_hook = $module . '_' . $name . '_chunk';
    if (!function_exists($this->form_id)) {
      watchdog('chunks', 'The %module module does not provide a configuration form for the %name chunk type.', array('%module' => $this->module, '%name' => $this->name), WATCHDOG_ERROR);
    }
    if (!function_exists('theme_' . $this->theme_hook)) {
      watchdog('chunks', 'The %module module does not provide a theme hook for the %name chunk type.', array('%module' => $this->module, '%name' => $this->name), WATCHDOG_ERROR);
    }
  }

  public function getForm($form, &$form_state, &$chunk_state) {
    $function = $this->form_id;
    return $function($form, $form_state, $chunk_state);
  }

  public function view($configuration) {
    return theme($this->theme_hook, $configuration);
  }
}

